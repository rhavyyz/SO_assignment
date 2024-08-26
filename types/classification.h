#pragma once





/*
    A class used to pass to the client information about its rank
*/
class Classification
{
private:
    /* data */
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
    ~Classification();
};
