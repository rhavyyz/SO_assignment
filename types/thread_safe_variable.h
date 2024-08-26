#pragma once

template<class T>
class ThreadSafeVariable
{
    private:
        T value;
        std::binary_semaphore sem{0};

    public:
        ThreadSafeVariable(T value)
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