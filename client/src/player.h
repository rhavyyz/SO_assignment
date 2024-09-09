#pragma once
#include "connection.h"
#include "shared_memory.h"
#include "registration.h"
#include <string>

#include <ctime>
#include <random>


class Player
{
    public: 
        int connection_id;
        Connection * connection;
        bool simulation = false;
        std::random_device seed;
        std::mt19937 rng= std::mt19937(seed());


        Player(int argc, char ** argv)
        {
            int registry_id = std::atoi(argv[1]);
            if (argc >= 3 && std::string(argv[2]) == "-s")
                simulation = true;
        

            Registration * registry= (Registration*)shared_memory::get_shared_memory(registry_id) ;
        
            registry->sem.acquire();

            if(!registry->openned)
            {
                registry->sem.release();
                return;
            }


            registry->client_id = getpid();

            registry->from_client_signal.send_signal();
            registry->from_server_signal.wait_signal();

            connection_id = registry->connection_memory_id;
            
            connection = ((Connection*)shared_memory::get_shared_memory(connection_id)) + registry->in_server_id;

            registry = NULL;
            shared_memory::free_shared_memory(registry_id);

        }

        void free()
        {
            shared_memory::free_shared_memory(connection_id);
        }

        void run()
        {
            while(true)
            {
                connection->from_server_signal.wait_signal();

                connection->question_view.display();

                int q_id = connection->question_view.quesiton_id.get_value(),
                    qtd_alt = connection->question_view.alternatives_qtd.get_value();

                connection->question_view.free();

                if (simulation)
                {
                    std::uniform_int_distribution<int> gen(0, qtd_alt-1);
                    connection->question_awnser.awnser_index.set_value(gen(rng));
                    connection->question_awnser.awnser_time.set_value(time(NULL));
                    connection->question_awnser.question_id.set_value(q_id);
                }
                else
                {
                    std::cout << "Write the number of your awnser" << std::endl;
                    int aux; std::cin >> aux;
                    connection->question_awnser.awnser_index.set_value(aux);
                    connection->question_awnser.awnser_time.set_value(time(NULL));
                    connection->question_awnser.question_id.set_value(q_id);
                }
                
                connection->from_server_signal.wait_signal();

                connection->classification.display();

                if(connection->classification.is_final)
                    break;
            }
            connection->free();
        }
};