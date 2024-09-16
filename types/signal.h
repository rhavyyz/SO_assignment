#pragma once

#include<semaphore>



/*
    Class to simulate a signal sent through threads/processes
*/
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
