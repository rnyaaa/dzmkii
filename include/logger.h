#ifndef _LOGGER_H
#define _LOGGER_H

#include <cstdio>

#define RESET_COLOR 	"\x1B[0m"
#define INFO_COLOR 	    "\x1B[1;34m"
#define DEBUG_COLOR 	"\x1B[1;35m"
#define WARNING_COLOR 	"\x1B[1;33m"
#define ERROR_COLOR 	"\x1B[31m"

namespace Log {

    template<typename ... Args>
    void info(const char *s, Args ... args )
    {
        printf("%s[INFO]: ", INFO_COLOR);
        printf(s, args...);
        printf("%s\n", RESET_COLOR);
    }

    template<typename ... Args>
    void debug(const char *s, Args ... args )
    {
        printf("%s[DEBUG]: ", DEBUG_COLOR);
        printf(s, args...);
        printf("%s\n", RESET_COLOR);
    }

    template<typename ... Args>
    void warning(const char *s, Args ... args )
    {
        printf("%s[WARNING]: ", WARNING_COLOR);
        printf(s, args...);
        printf("%s\n", RESET_COLOR);
    }

    template<typename ... Args>
    void error(const char *s, Args ... args )
    {
        printf("%s[ERROR]: ", ERROR_COLOR);
        printf(s, args...);
        printf("%s\n", RESET_COLOR);
    }
    
};

#endif // _LOGGER_H
