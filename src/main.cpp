#include "ncurses_display.h"
#include "system.h"

#include <iostream>

int main() {
  System system;
  NCursesDisplay::Display(system);
  // std::vector<Process>& processes = system.Processes();
  // for (Process& proc : processes){
  //   std::cout << proc.Pid() << ": " << proc.CpuUtilization() << std::endl;
  // }
}