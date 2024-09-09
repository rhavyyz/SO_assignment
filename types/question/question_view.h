#pragma once
#include "question/question.h"
#include "shared_memory.h"
#include "shared_resource.h"
#include <stdexcept>
#include <iostream>

class QuestionView
{
    private:
        int text_id = -1;
        int alternatives_list_id = -1;
        // std::binary_semaphore sem{0};

    public:
        SharedResource<int> alternatives_qtd = SharedResource<int>(0);
        SharedResource<int> quesiton_id = SharedResource<int>(0);
 
        void load(Question & question)
        {
            if(alternatives_list_id != -1)
                this->free();
            this->quesiton_id.set_value(question.question_id);

            this->alternatives_qtd.set_value( question.alternatives.size());

            this-> alternatives_list_id= shared_memory::create_shared_memory(sizeof(int) * this->alternatives_qtd.get_value());

            int * ids_list = (int*)shared_memory::get_shared_memory(this->alternatives_list_id);

            for(int i = 0; i < this->alternatives_qtd.get_value(); i++)
            {
                ids_list[i] = shared_memory::create_shared_memory((question.alternatives[i].size() + 1) * sizeof(char));
                char * message = (char*)shared_memory::get_shared_memory(ids_list[i]);
                

                // Copy the content
                for(int j = 0; j <question.alternatives[i].size(); j++)
                    message[j] = question.alternatives[i][j];
            
                // Adds a \0
                message[question.alternatives[i].size()] = '\0';
            }

            this->text_id = shared_memory::create_shared_memory((question.text.size() + 1) * sizeof(char));
            char * text = (char*)shared_memory::get_shared_memory(this->text_id);
            

            // Copy the content
            for(int j = 0; j <question.text.size(); j++)
                text[j] = question.text[j];
        
            // Adds a \0
            text[question.text.size()] = '\0';
        }

        std::string get_text()
        {
            char * text= (char*)shared_memory::get_shared_memory(this->text_id);

            return std::string(text);
        }

        std::string get_alternative(int i)
        {
            if(i <0 || i >= alternatives_qtd.get_value())
                throw std::runtime_error(std::string("alternative index must be 0 <= i < ") + std::to_string(alternatives_qtd.get_value()));
                
            int * ids_list = (int*)shared_memory::get_shared_memory(this->alternatives_list_id);
            char * message = (char*)shared_memory::get_shared_memory(ids_list[i]);

            return std::string(message);
        }

        void display()
        {
            std::cout << "Question " << quesiton_id.get_value() << ":"<< std::endl;
            std::cout << get_text() << std::endl << std::endl;
            for(int i =0; i < alternatives_qtd.get_value(); i++)
                std::cout << i << " - " << get_alternative(i) << std::endl;
        }

        void free()
        {
            int * ids_list = (int*)shared_memory::get_shared_memory(this->alternatives_list_id);

            for(int i = 0; i < this->alternatives_qtd.get_value(); i++)
                shared_memory::free_shared_memory(ids_list[i]);

            ids_list = NULL;

            shared_memory::free_shared_memory(this->alternatives_list_id);
            shared_memory::free_shared_memory(this->text_id);
        }
};