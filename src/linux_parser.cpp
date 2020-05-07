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

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream fstream(kOSPath);
  if (fstream.is_open()) {
    while (std::getline(fstream, line)) {
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
    fstream.close();
  }
  return value;
}

string LinuxParser::Kernel() {
  string os;
  string version;
  string kernel;
  string line;
  std::ifstream fstream(kProcDirectory + kVersionFilename);
  if (fstream.is_open()) {
    std::getline(fstream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
    fstream.close();
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

// Reads and returns the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  std::map<std::string, float> mem{{"MemTotal", 0},
                                   {"MemFree", 0},
                                   {"Buffers", 0},
                                   {"Cached", 0},
                                   {"Slab", 0}};

  std::ifstream fstream(kProcDirectory + kMeminfoFilename);
  if (fstream.is_open()) {
    std::map<std::string, float>::iterator it;
    while (std::getline(fstream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (mem.count(key) && !value.empty()) {
        mem[key] = atof(value.c_str());
      }
    }
    fstream.close();
    return (mem["MemTotal"] - mem["MemFree"] - mem["Buffers"] - mem["Cached"] -
            mem["Slab"]) /
           mem["MemTotal"];
  }
  return 0.0;
}

// Reads and returns the system uptime
long LinuxParser::UpTime() {
  return ParserHelper::getValue<long>(kProcDirectory + kUptimeFilename);
}

// Reads and returns the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ParserHelper::getValueByKey<long>(filterCpu,
                                           kProcDirectory + kStatFilename);
}

// Reads and returns the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::string line;
  std::string value;
  long activeJiffies = 0;
  std::ifstream fstream(kProcDirectory + "/" + to_string(pid) +
                           kStatFilename);
  if (fstream.is_open()) {
    std::getline(fstream, line);
    std::istringstream linestream(line);
    for (size_t i = 0; i <= 14; ++i) {
      linestream >> value;
      if ((i == 13) || (i == 14)) {
        activeJiffies += atol(value.c_str());
      }
    }
    fstream.close();
  }
  return activeJiffies;
}

// Reads and returns the number of active jiffies for a PID
long LinuxParser::ChildJiffies(int pid) {
  std::string line;
  std::string value;
  long childJiffies = 0;
  std::ifstream fstream(kProcDirectory + "/" + to_string(pid) +
                           kStatFilename);
  if (fstream.is_open()) {
    std::getline(fstream, line);
    std::istringstream linestream(line);
    for (size_t i = 0; i <= 14; ++i) {
      linestream >> value;
      if ((i == 15) || (i == 16)) {
        childJiffies += atol(value.c_str());
      }
    }
    fstream.close();
  }
  return childJiffies;
}

// Reads and returns the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  return getJiffySumByKeys({CPUStates::kUser_, CPUStates::kNice_,
                            CPUStates::kSystem_, CPUStates::kIRQ_,
                            CPUStates::kSoftIRQ_, CPUStates::kSteal_});
}

// Reads and returns the number of idle jiffies for the system
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

// Reads and returns CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> jiffies;
  string line;
  string cpu;
  string cpuTime;
  std::ifstream fstream(kProcDirectory + kStatFilename);
  if (fstream.is_open()) {
    std::getline(fstream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> cpuTime) {
      jiffies.push_back(cpuTime);
    }
    fstream.close();
  }
  return jiffies;
}

// Reads and returns the total number of processes
int LinuxParser::TotalProcesses() {
  return ParserHelper::getValueByKey<int>(filterProcesses,
                                          kProcDirectory + kStatFilename);
}

// Reads and returns the number of running processes
int LinuxParser::RunningProcesses() {
  return ParserHelper::getValueByKey<int>(filterRunningProcesses,
                                          kProcDirectory + kStatFilename);
}

// Reads and returns the command associated with a process
string LinuxParser::Command(int pid) {
  return ParserHelper::getValue<std::string>(kProcDirectory + "/" +
                                             to_string(pid) + kCmdlineFilename);
}

// Reads and returns the memory used by a process
// VmSize is the sum of all the virtual memory, see (http://man7.org/linux/man-pages/man5/proc.5.html)
// VmData gives the exact physical memory being used as a part of Physical RAM, that's why it's used here instead of VmSize
string LinuxParser::Ram(int pid) {
  return Format::kbToMb(ParserHelper::getValueByKey<int>(
      filterProcMem, kProcDirectory + "/" + to_string(pid) + kStatusFilename));
}

// Reads and returns the user ID associated with a process
string LinuxParser::Uid(int pid) {
  return ParserHelper::getValueByKey<string>(
      filterUID, kProcDirectory + "/" + to_string(pid) + kStatusFilename);
}

// Reads and returns the user associated with a process
string LinuxParser::User(int pid) {
  const string uid = LinuxParser::Uid(pid);
  std::ifstream fstream(kPasswordPath);
  std::string line;
  std::string username;
  std::string x;
  std::string userid;
  if (fstream.is_open()) {
    while (std::getline(fstream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> x >> userid) {
        if (userid == uid) {
          return username;
        }
      }
    }
    fstream.close();
    return username;
  }
  return string();
}

// Reads and returns the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream fstream(kProcDirectory + "/" + to_string(pid) +
                           kStatFilename);
  std::string line, value;
  size_t iter = 0;
  if (fstream.is_open()) {
    std::getline(fstream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      ++iter;
      if (iter == 22) {
        return atol(value.c_str());
      }
    }
    fstream.close();
  }
  return 0;
}