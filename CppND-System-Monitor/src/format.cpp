#include <string>
#include <sstream>
#include <iomanip>
#include "format.h"

using std::string;

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
    std::stringstream s;
    s << std::setfill('0') << std::setw(2) << ((seconds % 86400) / 3600) << ':'
      << std::setfill('0') << std::setw(2) << ((seconds % 3600) / 60) << ':'
      << std::setfill('0') << std::setw(2) << (seconds % 60);
    return s.str();
}