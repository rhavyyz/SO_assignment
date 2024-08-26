#pragma once
#include <thread>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <cstdlib>
#include <semaphore>

#include"message.h"
#include"classification.h"


/*
    Class that stores everything required to the comunication between server and client

    message_id - Stores the id to acess the shared memory to a Message class that is going
                 to be used for the comunication
    classification_id - Stores the id to acess the shared memory to a Classification class 
                        that is going to be use for the comunication

    client_id - stores the id from the client's thread and is used as a identifier within the 
                the server's business rules.

    from_client_signal - A semaphore that is going to be used as a signal from the client to the server
    from_server_signal - A semaphore that is going to be used as a signal from the client to the server
*/
class Connection
{
    public:
        std::thread::id client_id;
        std::thread::id server_id;
        int message_id;
        int classification_id;
        std::binary_semaphore from_client_signal{0};
        std::binary_semaphore from_server_signal{0};

        Connection(std::thread::id base_ids)
        {
            this->client_id = this->server_id = base_ids;

            this->message_id = shmget((key_t)1122, sizeof(Message), 0666 | IPC_CREAT);
            this->classification_id = shmget((key_t)1122, sizeof(Classification), 0666 | IPC_CREAT);
        }

        void free()
        {
            shmctl(this->message_id, IPC_RMID, NULL);
            shmctl(this->classification_id, IPC_RMID, NULL);
        // return true;   
        }    

};