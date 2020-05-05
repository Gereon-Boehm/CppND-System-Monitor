#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "linux_parser.h"

class Processor {
 public:
  float Utilization();

 private:
  float total = 0;
  float idleAll = 0;
};

#endif