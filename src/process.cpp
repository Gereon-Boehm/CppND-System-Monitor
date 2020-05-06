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
  starttime = float(LinuxParser::UpTime(pid));
}

// Returns this process's ID
int Process::Pid() const { return pid; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() const {
  const float total_time = float(LinuxParser::ActiveJiffies(pid)) +
                           float(LinuxParser::ChildJiffies(pid));
  const long hertz = sysconf(_SC_CLK_TCK);
  const float seconds = float(UpTime());
  return 100.0 * ((total_time / hertz) / seconds);
}

// TODO: Return the command that generated this process
string Process::Command() const { return LinuxParser::Command(pid); }

// TODO: Return this process's memory utilization
string Process::Ram() const { return LinuxParser::Ram(pid); }

// TODO: Return the user (name) that generated this process
string Process::User() const { return LinuxParser::User(pid); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() const {
  return LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
}

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  if (CpuUtilization() < a.CpuUtilization()) {
    return true;
  } else {
    return false;
  }
}