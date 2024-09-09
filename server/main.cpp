// #include <iostream>
// #include <thread>
// #include <sys/ipc.h>
// #include <cstdlib>
// #include <sys/shm.h>
// #include <utility>
// #include <semaphore>
// #include <chrono>
// #include <string.h>
// #include <vector>
// #include "connection.h"
// #include <algorithm>


// typedef vector<pair<int, thread::id>> scores_type;

#include <iostream>
#include "shared_resource.h"
#include "connection.h"
#include "question/question_awnser.h"
#include "question/question_view.h"
// #include "question/question_view.h"
#include "connection.h"
#include "game.h"


#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

using namespace std;

int main(int argc, char ** argv)
{
    Game (argc, argv).run();

    return 0;
}