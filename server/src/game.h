#pragma once
#include <stdexcept>

#include <string>
#include "json.hpp"
#include <fstream>
#include<vector>
#include "question/question.h"
#include "shared_memory.h"
#include "connection.h"
#include "registration.h"
#include <semaphore>
#include "game_state.h"
#include <unistd.h>
#include<utility>

using json = nlohmann::json;

using std::cout, std::endl;

class Game
{
    std::vector<Question> questions;
    Connection * connections;
    Registration * registration;
    int registration_id, connections_id;
    int max_players, time_for_question, current_players;

    GameState state = GameState::transitioning;
    std::binary_semaphore sem{1};

    std::vector<std::pair<int, int>> scores;

    void register_thread()
    {

        for(int i = 0; i < max_players; i++)
        {
            registration->from_client_signal.wait_signal();



            if (state != GameState::registration || !registration->openned)
            {
                registration->openned = false;
                shared_memory::free_shared_memory(registration_id);
                registration->from_server_signal.send_signal();
                sem.release();
                return;
            }

            new (connections+i) Connection(registration->client_id, getpid());
            current_players = i+1;

            scores.push_back({0, i});
            registration->in_server_id = i;

            if(i == max_players-1)
                registration->openned = false;

            registration->from_server_signal.send_signal();
        }
    }

    void registration_phase()
    {
        registration_id = shared_memory::create_shared_memory(sizeof(Registration));
    
        this->registration = (Registration*) shared_memory::get_shared_memory(registration_id);

        new (registration) Registration(-1, connections_id);

        cout << "Register your clients with the following number" << endl;
        cout << registration_id << endl;

        std::thread r (&Game::register_thread, this);

        cout << "Waiting for registrations" << endl << endl;
        cout << "Press ENTER if you want to end the registration" << endl;

        char c = std::getchar();

        registration->sem.acquire();
        state = GameState::transitioning;
        registration->from_client_signal.send_signal();
        registration->from_server_signal.wait_signal();

    }

    void launch_question(Question & question)
    {
        for(int i = 0; i < current_players; i++)
        {        
            connections[i].question_view.load(question);

            connections[i].from_server_signal.send_signal();
        }

        state = GameState::transitioning;
    }

    void launch_ranks(int question_id)
    {
        for (int i = 0; i < scores.size(); i++)
        {
                //    int total_players,
                //    int your_rank,
                //    int your_points,
                //    bool is_final
            connections[scores[i].second].classification = Classification(current_players, 
                                                                          i+1, 
                                                                          scores[i].first, 
                                                                          questions.back().question_id == question_id

            );
        }
        
    }

    void att_scores(Question & question)
    {
        std::vector<std::pair<time_t, std::pair<int, int>>> awnsers;

        for(int i = 0; i < current_players; i++)
        {
            if(connections[i].question_awnser.question_id.get_value() == question.question_id)
            {
                awnsers.push_back({
                    connections[i].question_awnser.awnser_time.get_value(),
                    {connections[i].question_awnser.awnser_index.get_value(), i}
                });
            }
        }


        std::sort(awnsers.begin(), awnsers.end());

        for(auto [t, awnser] : awnsers)
        {
            if(awnser.first == question.awnser_index)
            {
                for(auto & score : scores)
                {
                    if (score.second == awnser.second)
                    {
                        score.first++;
                        break;
                    }
                }

                std::stable_sort(scores.begin(), scores.end());

                break;
            }
        }
    }

    void free()
    {
        shared_memory::free_shared_memory(connections_id);
        shared_memory::free_shared_memory(registration_id);
    }

    public:
        Game(int argc, char ** argv)
        {
            if (argc < 2)
                throw std::runtime_error("You must pass the questions json as a argument to the server program!");
            
            std::ifstream f(argv[1]);
            json file = json::parse(f);

            this->max_players = file["max_players"].get<int>();
            this->time_for_question = file["time_for_question"].get<int>();

            connections_id = shared_memory::create_shared_memory(max_players*sizeof(Connection));
            connections = (Connection*) shared_memory::get_shared_memory(connections_id);

            json data = file["questions"];

            for (size_t i = 0; i < data.size(); i++)
            {

                std::vector<std::string> alt;

                for(int j = 0; j < data[i]["alternatives"].size(); j++)
                    alt.push_back(
                        data[i]["alternatives"][j].get<std::string>()
                    );

                questions.push_back(
                    Question(
                        data[i]["text"].get<std::string>(), 
                        alt,
                        data[i]["awnser_index"].get<int>(),
                        i
                    )
                );
            }
        }

        void run()
        {
            state = GameState::registration;
            registration_phase();

            for(auto &question : questions)
            {
                state = GameState::launching_question;
                launch_question(question);
                
                state = GameState::waiting_for_awnser;
                sleep(time_for_question);
                
                att_scores(question);

                state = GameState::launching_ranks;
                launch_ranks(question.question_id);

            }


            for(auto score : scores)
                cout << "player id: " << connections[score.second].client_id << " , scores: " << score.first << endl;

            this->free();
        }

};