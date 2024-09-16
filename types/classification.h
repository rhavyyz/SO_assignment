#pragma once
#include <iostream>

/*
    A class used to pass to the client information about its ranking
*/
class Classification
{
public:
    int total_players;
    int your_rank;
    int your_points;
    bool is_final;

    Classification(
                   int total_players,
                   int your_rank,
                   int your_points,
                   bool is_final
                  )
    {
        this->is_final=is_final;
        this->total_players=total_players;
        this->your_points=your_points;
        this->your_rank=your_rank;
    }

    void display()
    {
        std::cout << "Your "<< (is_final ? "final rank" : "rank") << " is:" << std::endl;
        std::cout << "Of " << total_players << " you are the " << your_rank << "th one" << std::endl;
        std::cout << "Points: " << your_points << std::endl << std::endl;
    }
};
