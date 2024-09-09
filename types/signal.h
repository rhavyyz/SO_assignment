#pragma once

#include<semaphore>

class Signal
{
private:
    std::binary_semaphore sem{0};
    
public:
    void wait_signal()
    {
        this->sem.acquire();
    }

    void send_signal()
    {
        this->sem.release();
    }

};
