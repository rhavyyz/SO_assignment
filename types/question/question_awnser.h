#pragma once
#include "shared_resource.h"
#include<ctime>

class QuestionAwnser
{
    public:

        SharedResource<int> awnser_index = SharedResource<int>(0), 
                            question_id = SharedResource(-1);
        SharedResource<time_t> awnser_time = SharedResource<time_t>(-1);
    
};