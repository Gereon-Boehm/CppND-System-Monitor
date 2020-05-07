#ifndef PARSER_HELPER_H
#define PARSER_HELPER_H

#include <fstream>
#include <string>

namespace ParserHelper{
template <typename T>
T getValueByKey(const std::string& filter, const std::string& filename) {
  std::string line, key;
  T value;
  std::ifstream stream(filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == filter) {
          return value;
        }
      }
    }
    stream.close();
  }
  return value;
};

template <typename T>
T getValue(const std::string& filename) {
  std::string line, key;
  T value;
  std::ifstream stream(filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
    stream.close();
  }
  return value;
};
}

#endif