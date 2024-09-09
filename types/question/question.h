#pragma once

#include<string>
#include<vector>

class Question
{
public:
    std::string text;
    std::vector<std::string> alternatives;
    int awnser_index;
    int question_id;
 
    Question(std::string text, const std::vector<std::string>& alternatives, int awnser_index, int question_id)
    {
        this-> text = text;
        this-> alternatives = alternatives;
        this-> awnser_index = awnser_index;
        this->question_id=question_id;
    }
    
};
