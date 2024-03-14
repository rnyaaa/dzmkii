#include "logger.h"

bool _logger_verbose = false;

namespace Log 
{

    void setLogLevel(LogLevel level)
    {
        _logger_verbose =  (level == LogLevel::VERBOSE);
    }
   
}
