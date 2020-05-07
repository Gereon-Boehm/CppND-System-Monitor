#include <iomanip>
#include <string>
#include <sstream>

#include "format.h"

using std::string;

string Format::Format(int time){
  const std::string timeAsString = std::to_string(time);
  return std::string(2 - timeAsString.length(), '0') + timeAsString;
}

// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  int minutes = seconds / 60;
  int hours = minutes / 60;
  minutes %= 60;
  seconds %= 60;
  return Format(hours) + ":" + Format(minutes) + ":" + Format(int(seconds));
}

string Format::kbToMb(const int kb){
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << (kb / 1024.0);
  return stream.str();
}