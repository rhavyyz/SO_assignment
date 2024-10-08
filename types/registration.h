#pragma once
#include <semaphore>
#include <thread>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <cstdlib>

#include<unistd.h>

#include "signal.h"

#include"classification.h"
// #include "thread_safe_variable.py"


/*
    This class is going to be used to pass informations about the client
    to the server, so it can register a client. The server also provides
    the information about the shared memory in which the comunication is
    going to occour. There is going to be only one Registration point so
    to start the registration the client need to pass throught the semaphore
*/
class Registration
{
    public:
        pid_t client_id;
        int in_server_id;
        int connection_memory_id;

        std::binary_semaphore sem{0};

        Signal from_client_signal, 
               from_server_signal;

        bool openned = true;

        Registration(int in_server_id, int connection_memory_id)
        {

            this->in_server_id=in_server_id;
            this->connection_memory_id = connection_memory_id;

            sem.release();
        }

    

};