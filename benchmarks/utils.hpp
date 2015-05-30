#include <ctime>
#include <map>
#include <string>
#include <chrono>
#include <iostream>


using chrono_t = std::chrono::duration<double>;
std::map<std::string, chrono_t> _timer_global;


#define START_TIMER( section, group ) \
        _timer_global.emplace(std::pair<std::string, chrono_t>(group, {})); \
        std::string _timer_group = group; \
        std::cout << "> " << section << std::endl; \
        std::chrono::time_point<std::chrono::system_clock> _timer_start, _timer_end; \
        _timer_start = std::chrono::system_clock::now();

#define END_TIMER() \
        _timer_end = std::chrono::system_clock::now(); \
        chrono_t elapsed_seconds = _timer_end - _timer_start; \
        _timer_global[_timer_group] += elapsed_seconds; \
        std::cout << "< Seconds: " << elapsed_seconds.count() << std::endl; \
        std::cout << std::endl;

#define TIMER_STATS() \
        chrono_t _total_seconds {}; \
        for (const auto & entry : _timer_global) { \
            const auto & group = entry.first; \
            const auto & elapsed_seconds = entry.second; \
            std::cout << "Group " << group << ". "; \
            std::cout << "Seconds: " << elapsed_seconds.count() << std::endl; \
            _total_seconds += elapsed_seconds; \
        } \
        std::cout << "-----" << std::endl; \
        std::cout << "Total seconds: " << _total_seconds.count() << std::endl; \
        std::cout << std::endl;