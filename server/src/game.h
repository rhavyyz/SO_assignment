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

// #define DEBUG 1

using json = nlohmann::json;

using std::cout, std::endl;

class Game
{
    std::vector<Question> questions;
    Connection * connections;
    Registration * registration;
    int registration_id, connections_id;
    int max_players, time_for_question, current_players= 0;

    GameState state = GameState::transitioning;
    std::binary_semaphore sem{1};

    std::vector<std::pair<int, int>> scores;

    void register_thread()
    {

        for(int i = 0; i < max_players; i++)
        {
            #ifdef DEBUG
                std::cout << "[DEBUG] Waiting for someone to start its registration" << std::endl;
            #endif  
            registration->from_client_signal.wait_signal();
            #ifdef DEBUG
                std::cout << "[DEBUG] Started registrating" << std::endl;
            #endif  

            if (state != GameState::registration || !registration->openned)
            {
                #ifdef DEBUG
                    std::cout << "[DEBUG] Registration ending conditions where reached" << std::endl;
                #endif  

                registration->openned = false;
                registration->from_server_signal.send_signal();
                return;
            }

            new (connections+i) Connection(registration->client_id, getpid());
            current_players = i+1;

            scores.push_back({0, i});
            registration->in_server_id = i;

            if(i == max_players-1)
                registration->openned = false;

            #ifdef DEBUG
                std::cout << "[DEBUG] Infos about the current registration" << std::endl;
                std::cout << "[DEBUG] " << registration->client_id << std::endl;
                std::cout << "[DEBUG] " << registration->connection_memory_id << std::endl;
                std::cout << "[DEBUG] " << registration->in_server_id << std::endl;
                std::cout << "[DEBUG] " << registration->openned << std::endl;
            #endif  


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
        if (registration->openned)
        {    
            #ifdef DEBUG
                std::cout << "[DEBUG] simulating the client behavior to induce the connection thread do end properly" << std::endl;
            #endif  

            registration->from_client_signal.send_signal();
            registration->from_server_signal.wait_signal(); 
        }
        registration = NULL;
        r.join();
        shared_memory::free_shared_memory(registration_id);  
    }

    void launch_question(Question & question)
    {
        #ifdef DEBUG
            std::cout << "[DEBUG] Starting to load questions" << std::endl;
        #endif
        for(int i = 0; i < current_players; i++)
        {        

            #ifdef DEBUG
                std::cout << "[DEBUG] Setting question " << question.question_id<< " to " << i << "th " << std::endl;
            #endif
            connections[i].question_view.load(question);

            #ifdef DEBUG
                std::cout << "[DEBUG] Sending signal to the " << i << "th client" << std::endl;
            #endif
            connections[i].from_server_signal.send_signal();
        }


        #ifdef DEBUG
            std::cout << "[DEBUG] Ended sending the question " << question.question_id << std::endl;
        #endif

        state = GameState::transitioning;
    }

    void launch_ranks(int question_id)
    {
        for (int i = 0; i < scores.size(); i++)
        {
            #ifdef DEBUG
                std::cout << "[DEBUG] Setting classification to " << i << "th " << std::endl;
            #endif

            connections[scores[i].second].classification = Classification(current_players, 
                                                                          i+1, 
                                                                          scores[i].first, 
                                                                          questions.back().question_id == question_id

            );


            #ifdef DEBUG
                std::cout << "[DEBUG] Sending signal to the " << i << "th client" << std::endl;
            #endif
            connections[scores[i].second].from_server_signal.send_signal();
        }
    }

    void update_scores(Question & question)
    {
        std::vector<std::pair<time_t, std::pair<int, int>>> awnsers;


        #ifdef DEBUG
            cout << "[DEBUG] Getting awnsers" << endl;
        #endif  
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

        #ifdef DEBUG
            cout << "[DEBUG] Sorting awnser by time " << endl;
        #endif  
        std::sort(awnsers.begin(), awnsers.end());


        for(auto [t, awnser] : awnsers)
        {
            
            if(awnser.first == question.awnser_index)
            {
                #ifdef DEBUG
                    cout << "[DEBUG] First correct awnser sent by the " << awnser.second << "th connection" << endl;
                #endif  
                for(auto & score : scores)
                {

                    #ifdef DEBUG
                        cout << "it entered" << endl;
                    #endif  

                    if (score.second == awnser.second)
                    {
                        score.first++;

                        cout << std::endl;
                        break;
                    }
                }

                #ifdef DEBUG
                    cout << "[DEBUG] Sorting new scores" << endl;
                #endif 
                std::stable_sort(scores.begin(), scores.end(), std::greater<std::pair<int, int>>());

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
            
            #ifdef DEBUG
                cout << "[DEBUG] Reading json info" << endl;
            #endif 

            this->max_players = file["max_players"].get<int>();
            this->time_for_question = file["time_for_question"].get<int>();

            connections_id = shared_memory::create_shared_memory(max_players*sizeof(Connection));
            connections = (Connection*) shared_memory::get_shared_memory(connections_id);

            json data = file["questions"];

            for (size_t i = 0; i < data.size(); i++)
            {

                #ifdef DEBUG
                    cout << "[DEBUG] Reading " << i << "th question" << endl;
                #endif 


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

            #ifdef DEBUG
                cout << "[DEBUG] Start post registration" << endl;
            #endif  

            for(auto &question : questions)
            {
                #ifdef DEBUG
                    cout << "[DEBUG] Sending question" << endl;
                #endif  
                state = GameState::launching_question;
                launch_question(question);
                
                state = GameState::waiting_for_awnser;
                sleep(time_for_question);
            
                #ifdef DEBUG
                    std::cout << "[DEBUG] Getting the awnsers and recalculating the scores" << std::endl;
                #endif                  
                update_scores(question);
                #ifdef DEBUG
                    std::cout << "[DEBUG] Printing the scores for each registrated client" << std::endl;
                    for(const auto & score: scores)
                        std::cout << "[DEBUG] " << score.first << ' ' << score.second << std::endl;


                    std::cout << "[DEBUG] Getting the awnsers and recalculating the scores" << std::endl;
                #endif  

                state = GameState::launching_ranks;
                launch_ranks(question.question_id);
                #ifdef DEBUG
                    cout << "[DEBUG] Ended the process to the " << question.question_id << "th question" << endl;
                #endif  
            }


            for(auto score : scores)
                cout << "player id: " << connections[score.second].client_id << " , scores: " << score.first << endl;

            this->free();
        }

};