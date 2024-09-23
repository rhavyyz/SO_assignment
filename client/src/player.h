#pragma once
#include "connection.h"
#include "shared_memory.h"
#include "registration.h"
#include <string>
#include <iostream>
#include <ctime>
#include <random>

typedef std::uniform_int_distribution<int> int_gen;

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

            // just setting up the execution variables
            int registration_id = std::atoi(argv[1]);
            if (argc >= 3 && std::string(argv[2]) == "-s")
                simulation = true;

            Registration * registration= (Registration*)shared_memory::get_shared_memory(registration_id) ;        

            // 
            registration->sem.acquire();

            if(!registration->openned)
            {
                #ifdef DEBUG
                    std::cout << "[DEBUG] Terminating the initialization if isnt open" << std::endl; 
                #endif
                registration->sem.release();
                return;
            }

            #ifdef DEBUG
                std::cout << "[DEBUG] Setting the client_id variable" << std::endl; 
            #endif
            registration->client_id = getpid();


            #ifdef DEBUG
                std::cout << "[DEBUG] Sending a signal to the server so it can set up the values for this registration" << std::endl; 
            #endif
            registration->from_client_signal.send_signal();


            #ifdef DEBUG
                std::cout << "[DEBUG] Waiting till it end the registration values setup" << std::endl; 
            #endif
            registration->from_server_signal.wait_signal();
            
            #ifdef DEBUG
                std::cout << "[DEBUG] Storing connection data" << std::endl;
            #endif  
            connection_id = registration->connection_memory_id;            
            connection = ((Connection*)shared_memory::get_shared_memory(connection_id)) + registration->in_server_id;



            #ifdef DEBUG
                std::cout << "[DEBUG] Releasing registration shared resources" << std::endl; 
            #endif
            registration->sem.release();
            shared_memory::free_shared_memory(registration_id);
            registration = NULL;

        }

        void free()
        {
            shared_memory::free_shared_memory(connection_id);
        }

        void run()
        {
            std::cout << "Running with code number " << getpid() << std::endl;
            while(true)
            {
                connection->from_server_signal.wait_signal();
                #ifdef DEBUG
                    std::cout << "[DEBUG] Received signal to read question" << std::endl; 

                    std::cout << "[DEBUG] Displaying question and getting question information" << std::endl; 
                #endif    

                connection->question_view.display();

                int q_id = connection->question_view.quesiton_id.get_value(),
                    qtd_alt = connection->question_view.alternatives_qtd.get_value();

                connection->question_view.free();


                #ifdef DEBUG
                    std::cout << "[DEBUG] Generating/getting awnser and saving in connection point" << std::endl; 
                #endif  
                int awnser_value;
                if (simulation)
                {
                    awnser_value =  int_gen(0, qtd_alt-1)(rng);
                    std::cout << "Guessed the item " << awnser_value << std::endl;
                }
                else
                {
                    std::cout << "Write the number of your awnser" << std::endl;
                    std::cin >> awnser_value;
                }
                connection->question_awnser.awnser_index.set_value(awnser_value);

                #ifdef DEBUG
                    std::cout << "[DEBUG] Converting time to a readable time format" << std::endl; 
                #endif  

                auto now = time(NULL);
                char readable_now[80]; 
                strftime (readable_now,80,"%H:%M:%S",localtime(&now));    

                std::cout << "awnser sent to server at " << readable_now<<  " | integer representation : " << now << std::endl ;

                connection->question_awnser.awnser_time.set_value(now);
                connection->question_awnser.question_id.set_value(q_id);
                

                connection->from_server_signal.wait_signal();
                #ifdef DEBUG
                    std::cout << "[DEBUG] Received signal do display classification" << std::endl; 
                #endif  
                connection->classification.display();

                if(connection->classification.is_final)
                    break;
            }
            connection->free();
        }
};