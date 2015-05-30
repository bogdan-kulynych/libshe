#include <ctime>
#include <chrono>
#include <iostream>


#define START_TIMER( section ) \
        std::cout << "> " << section << std::endl; \
        std::chrono::time_point<std::chrono::system_clock> _timer_start, _timer_end; \
        _timer_start = std::chrono::system_clock::now(); \

#define END_TIMER() \
        _timer_end = std::chrono::system_clock::now(); \
        std::chrono::duration<double> elapsed_seconds = _timer_end - _timer_start; \
        std::cout << "< Secs: " << elapsed_seconds.count() << std::endl; \
        std::cout << std::endl;