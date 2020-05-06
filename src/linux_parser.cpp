#include <dirent.h>
#include <unistd.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "ParserHelper.h"
#include "format.h"
#include "linux_parser.h"

#include <iostream>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key, value;
  float memtotal = 0.0, memfree = 0.0, buffers = 0.0, cached = 0.0, slab = 0.0;
  std::ifstream memfstream(kProcDirectory+kMeminfoFilename);
  if(memfstream.is_open()){
    while(std::getline(memfstream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream stream(line);
      stream >> key >> value;
      if (key == "MemTotal") { memtotal = stof(value); }
      if (key == "MemFree") { memfree = stof(value); }
      if (key == "Buffers") { buffers = stof(value); }
      if (key == "Cached") { cached = stof(value); }
      if (key == "Slab") { slab = stof(value); }
      if (memtotal != 0 && memfree != 0 && buffers != 0 && cached != 0 && slab != 0){ break;}
    };
    return (memtotal - memfree - buffers - cached - slab)/memtotal;
  }
  return 0.0;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  return ParserHelper::getValue<long>(kProcDirectory + kUptimeFilename);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ParserHelper::getValueByKey<long>("cpu",
                                           kProcDirectory + kStatFilename);
}

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::string line, value;
  long activeJiffies = 0;
  std::ifstream filestream(kProcDirectory + "/" + to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (size_t i = 0; i <= 14; ++i) {
      linestream >> value;
      if ((i == 13) || (i == 14)) {
        activeJiffies += atol(value.c_str());
      }
    }
  }
  return activeJiffies;
}

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ChildJiffies(int pid) {
  std::string line, value;
  long childJiffies = 0;
  std::ifstream filestream(kProcDirectory + "/" + to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (size_t i = 0; i <= 14; ++i) {
      linestream >> value;
      if ((i == 15) || (i == 16)) {
        childJiffies += atol(value.c_str());
      }
    }
  }
  return childJiffies;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  return getJiffySumByKeys({CPUStates::kUser_, CPUStates::kNice_,
                            CPUStates::kSystem_, CPUStates::kIRQ_,
                            CPUStates::kSoftIRQ_, CPUStates::kSteal_});
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  return getJiffySumByKeys({CPUStates::kIdle_, CPUStates::kIOwait_});
}

long LinuxParser::getJiffySumByKeys(const std::vector<CPUStates>& keys) {
  vector<string> jiffies{CpuUtilization()};
  long sum = 0;
  for (const size_t key : keys) {
    sum += atol(jiffies[key].c_str());
  }
  return sum;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> jiffies;
  string line, cpu, cpuTime;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> cpuTime) {
      jiffies.push_back(cpuTime);
    }
  }
  return jiffies;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  return ParserHelper::getValueByKey<int>("processes",
                                          kProcDirectory + kStatFilename);
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return ParserHelper::getValueByKey<int>("procs_running",
                                          kProcDirectory + kStatFilename);
}

// TODO: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  return ParserHelper::getValue<std::string>(kProcDirectory + "/" +
                                             to_string(pid) + kCmdlineFilename);
}

// TODO: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  return Format::kbToMb(ParserHelper::getValueByKey<int>(
      "VmSize:", kProcDirectory + "/" + to_string(pid) + kStatusFilename));
}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  return ParserHelper::getValueByKey<string>(
      "Uid:", kProcDirectory + "/" + to_string(pid) + kStatusFilename);
}

// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  const string uid = LinuxParser::Uid(pid);
  std::ifstream filestream(kPasswordPath);
  std::string line, username, x, userid;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> x >> userid) {
        if (userid == uid) {
          return username;
        }
      }
    }
  }
  return username;
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream filestream(kProcDirectory + "/" + to_string(pid) +
                           kStatFilename);
  std::string line, value;
  size_t iter = 0;
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      ++iter;
      if (iter == 22) {
        return atol(value.c_str());
      }
    }
  }
  return 0;
}