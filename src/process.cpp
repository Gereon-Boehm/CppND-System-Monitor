#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"

#include <iostream>

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid(pid) {
  starttime = static_cast<float>(LinuxParser::UpTime(pid));
}

// Returns this process's ID
int Process::Pid() const { return pid; }

// Returns this process's CPU utilization
float Process::CpuUtilization() const {
  const float total_time = static_cast<float>(LinuxParser::ActiveJiffies(pid)) +
                           static_cast<float>(LinuxParser::ChildJiffies(pid));
  const long hertz = sysconf(_SC_CLK_TCK);
  const float seconds = static_cast<float>(UpTime());
  return (total_time / hertz) / seconds;
}

// Returns the command that generated this process
string Process::Command() const {
  string command = LinuxParser::Command(pid);
  if (command.size() > MaxCommandLength) {
    command = command.substr(0, MaxCommandLength) + "...";
  }
  return command;
}

// Returns this process's memory utilization
string Process::Ram() const { return LinuxParser::Ram(pid); }

// Returns the user (name) that generated this process
string Process::User() const { return LinuxParser::User(pid); }

// Returns the age of this process (in seconds)
long int Process::UpTime() const {
  return LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
}

// Overloads the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  if (CpuUtilization() < a.CpuUtilization()) {
    return true;
  } else {
    return false;
  }
}