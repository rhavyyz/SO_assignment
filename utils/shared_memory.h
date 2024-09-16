#pragma once
#include <sys/shm.h>
#include <sys/ipc.h>
// #include <cstdlib>

namespace shared_memory
{

    int key = 1122;
    int create_shared_memory(int size)
    {
        return shmget((key_t)key++, size, 0666 | IPC_CREAT);
    }

    void* get_shared_memory(int id)
    {
        return shmat(id, nullptr, 0);
    }

    void free_shared_memory(int id)
    {
        shmctl(id, IPC_RMID, nullptr);
    }
};