#pragma once
#include <thread>
#include <string>
#include <sys/ipc.h>
#include <cstdlib>
#include <sys/shm.h>
#include <utility>
#include <semaphore> 
#include <chrono>
#include<time.h>



/*
Class that represents a string message passed client<->server
*/
class Message
{
    private:
        int id;
        int size;
        std::thread::id sender_id;
        bool ok = false;
        std::binary_semaphore sem{0};
        bool read;
        std::time_t t;

    public:
        Message(std::string value)
        {
            this->t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            this->read = false;
            this->ok = true;
            this->size = value.size() * sizeof(char);
            this->id = shmget((key_t)1122, this->size, 0666 | IPC_CREAT);;

            this->sender_id = std::this_thread::get_id();

            char * m = (char*) shmat(this->id, NULL, 0);

            for(int i = 0; i < value.size(); i++)
                m[i] = value[i];


            shmdt(m);
            this->sem.release();
        }


        std::pair<bool, std::string> get_message()
        {
            this->sem.acquire();
            std::string ans;
            bool ans_bool = ok;
            if (ans_bool)
            {
                this->read=true;
                char * m = (char*) shmat(this->id, NULL, 0);

                ans = std::string(m, this->size);

                shmdt(m);
            }

            this->sem.release();
            return {ans_bool, ans};
        }

        std::pair<bool, std::time_t> get_time()
        {
            this->sem.acquire();
            std::time_t ans=-1;
            bool ans_bool = ok;
            if (ans_bool)
            {
                this->read=true;

                ans = this->t;
            }

            this->sem.release();
            return {ans_bool, ans};
        }
        std::pair<bool, std::thread::id> get_sender_id()
        {
            this->sem.acquire();
            std::thread::id ans;
            bool ans_bool = ok;
            if (ans_bool)
            {
                this->read=true;

                ans = this->sender_id;
            }

            this->sem.release();
            return {ans_bool, ans};
        }

        bool was_read()
        {
            this->sem.acquire();
            bool ans = this->read;
            this->sem.release();
            return ans;
        }

        bool is_ok()
        {
            this->sem.acquire();
            bool ans = this->ok;
            this->sem.release();
            return ans;
        }

        void free()
        {
            shmctl(this->id, IPC_RMID, NULL);
            // return true;   
        }
        ~Message()
        {
            // this->free();
        }

};
