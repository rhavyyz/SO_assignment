#include "message.h"
#include <iostream>
#include <thread>
#include <sys/ipc.h>
#include <cstdlib>
#include <sys/shm.h>
#include <utility>
#include <semaphore>
#include <chrono>
#include <string.h>
#include <vector>
#include "connection.h"
#include <algorithm>
#include "phases.h"

typedef vector<pair<int, thread::id>> scores_type;

using namespace std;


void add_score_to(thread::id id, scores_type & scores)
{
    for(auto && [points, client_id] : scores)
    {
        if(client_id != id)
            continue;

        points++;

        stable_sort(scores.begin(), scores.end());
        break;
    }
}

int main()
{
    int n; 
    cout << "How many people should be in the quiz?"<< endl;
    cin >> n;
    cout << "Setting up..." << endl;

    int connections_id = shmget((key_t)1122, sizeof(Connection)*n, 0666 | IPC_CREAT);
    Connection* connections = (Connection*) shmat(connections_id, NULL, 0);

    shmctl(connections_id, IPC_RMID, NULL);


    for(int i = 0; i < n; i++)
        new (connections+i) Connection(std::this_thread::get_id());

    int total_allocated = n;


    thread* t = new thread(registration_phase, n, connections);

    this_thread::sleep_for(chrono::seconds(3));

    cout << endl << endl << "Press any ENTER if you want to end the registrations" << endl;
    char _ =getchar();
    delete t;

    scores_type scores;
    for(int i = 0; i < n; i++)
    {
        if(connections[i].client_id == connections[i].server_id)
        {
            n = i;
            break;
        }
        scores.push_back({0, connections[i].client_id});
    }

    int q; 
    cout << "How many questions ?"<< endl;
    cin >> q;
    cout << "Now write all " << q << " questions" << endl;

    vector<string> questions(q);
    for(auto & question :questions )
        getline(cin, question);

    int seconds; 
    cout << "How many second you want to give for each question ?"<< endl;
    cin >> seconds;

    for(auto question : questions)
    {

        delivery_phase(question, n , connections);

        this_thread::sleep_for(chrono::seconds(seconds));

        vector<std::pair<thread::id, std::string>> awnsers = collection_phase(n, connections);

        for(auto [client_id, awnser] : awnsers)
        {
            cout << "Is the following awnser correct? (Type \"yes\" if it is. Any other awnser is going to be considered no)" << endl << endl;
            cout << '"' << awnser << '"' << endl << endl;
            cin >> awnser;
            if(awnser == "yes")
            {
                add_score_to(client_id, scores);
                break;
            }
        }


        notify_phase(false, n, connections);
    }

    notify_phase(true, n, connections);

    for(int i =0; i < total_allocated; i++)
        connections[i].free();

    shmdt(connections);
    return 0;
}