#include<thread>
class message
{
public:
 
    std::thread::id sender_id;
    message(/* args */);
    ~message();
};
