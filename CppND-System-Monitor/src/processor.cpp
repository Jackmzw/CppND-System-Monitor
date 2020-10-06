#include "processor.h"

// Return the aggregate CPU utilization
float Processor::Utilization() const{
  return LinuxParser::CpuUtilization(); 
}