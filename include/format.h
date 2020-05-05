#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
std::string Format(int time);

std::string ElapsedTime(long times);  // TODO: See src/format.cpp

std::string kbToMb(int kb);
};                                    // namespace Format

#endif