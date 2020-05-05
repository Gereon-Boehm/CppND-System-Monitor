#include "processor.h"

#include <string>
#include <vector>

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  float user, nice, system, idle, iowait, irq, softirq, steal,
      guest, guest_nice;
  const float prevTotal = total;
  const float prevIdleAll = idleAll;
  std::vector<std::string> cpuTimes = LinuxParser::CpuUtilization();
  std::stringstream ss;

  // Populate
  std::copy(cpuTimes.begin(), cpuTimes.end(),
            std::ostream_iterator<std::string>(ss, " "));
  ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >>
      guest >> guest_nice;

  total = user + nice + system + idle + iowait + irq + softirq + steal;
  idleAll = idle + iowait;

  const float totald = total - prevTotal;
  const float idled = idleAll - prevIdleAll;

  return (totald - idled) / totald;
}