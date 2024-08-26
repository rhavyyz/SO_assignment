#include<set>
#include<map>
#include<thread>
#include<iostream>
#include <utility>
#include "registration_type.h"
#include "connection.h"

// Message* get_ptr(int i, Connection* c)
// {

// }

using namespace std;

void registration_phase(int n, Connection *c)
{

    int id = shmget((key_t)1122, sizeof(Registration), 0666 | IPC_CREAT);
    Registration* a = (Registration*) shmat(id, NULL, 0);

    shmctl(id, IPC_RMID, NULL);

    new (a) Registration();
    Registration& registration = (*a);

    std::cout << "Connect with the following code" << std::endl << id << std::endl;

    for(int i = 0; i < n; i++)
    {
        registration.from_client_signal.acquire();
        registration.from_client_signal.release();

        c[i].client_id= registration.client_id;

        registration.in_server_id = i;
        registration.memory_id = id;

        registration.from_server_signal.release();

    }

    shmdt(a);

}


std::set<int> to_ignore;
int q = 0;
void delivery_phase(string question, int n, Connection* c)
{
    for (size_t i = 0; i < n; i++)
    {
        if(to_ignore.count(i))
            continue;

        if(q++ && !c[i].from_client_signal.try_acquire())
            to_ignore.insert(i);
        

        Message* ptr= (Message*) shmat(c[i].message_id, NULL, 0);
        if(ptr->is_ok())
            ptr->free();

        // creates the new message
        new (ptr) Message(question);
        shmdt(ptr);

        // Notifies the client that should be waiting to display the message
        c[i].from_server_signal.release();

    }  
}


vector<std::pair<thread::id, std::string>> collection_phase(int n, Connection* c)
{
    vector<pair<long, pair<thread::id, std::string>>> ans;

    for (size_t i = 0; i < n; i++)
    {
        if(to_ignore.count(i))
            continue;

        if(q++ && !c[i].from_client_signal.try_acquire())
        {
            to_ignore.insert(i);
            continue;
        }
        

        Message* ptr= (Message*) shmat(c[i].message_id, NULL, 0);
        
        auto m = ptr->get_message();

        if(!m.first || ptr->was_read())
        {
            to_ignore.insert(i);
            continue;
        }

        ans.push_back({ptr->get_time().second, {ptr->get_sender_id().second, m.second}});

        // Notifies the client that should be waiting to display the message
        c[i].from_server_signal.release();

    }  



    sort(ans.begin(), ans.end());


    vector<std::pair<thread::id, std::string>> formated;
    for (auto [f, s] : ans)
        formated.push_back(s);

    return formated;
    

}