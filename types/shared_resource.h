#pragma once

#include<semaphore>
// #include"shared_memory.h"

template<class T>
class SharedResource
{
    private:
        T value;
        std::binary_semaphore sem{0};

    public:
        SharedResource(T value)
        {
            this->value = value;
            this->sem.release();
        }

        void set_value(T value)
        {
            this->sem.acquire();
            this->value = value;
            this->sem.release();
        }

        T get_value()
        {
            this->sem.acquire();
            T ans = value;
            this->sem.release();
            return ans;
        }
};