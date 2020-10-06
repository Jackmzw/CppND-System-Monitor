#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

template <typename T>
T findValueByKey(std::string const &keyFilter, std::string const &filename) {
  std::string line, key;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          stream.close();
          return value;
        }
      }
    }
    stream.close();
  }
  return value;
};

template <typename T>
T getValueOfFile(std::string const &filename) {
  std::string line;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
    stream.close();
  }
  return value;
};

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

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel >> version;
  }
  return version;
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

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string memTotal = "MemTotal:";
  string memFree = "MemFree:";
  float total = findValueByKey<float>(memTotal, kMeminfoFilename);// "/proc/memInfo"
  float free = findValueByKey<float>(memFree, kMeminfoFilename);
  return (total - free) / total;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  long upTime = getValueOfFile<long>(kUptimeFilename);
  return upTime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  std::string line;
  std::string key;
  CPUStates states{};
  long jiffies{0};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while(std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        linestream >> states.user >> states.nice >> states.system 
        >> states.idle >> states.iowait >> states.irq >> states.softirq 
        >> states.steal >> states.guest >> states.guestnice;
        jiffies = states.Total();
        return jiffies;
      }
    }
  }
  return jiffies;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string value;
  string utime, stime, cutime, cstime; 
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      for(int i = 0; i < 13; i++) {
        linestream >> value; // Stop at 13th token
      }
      linestream >> utime >> stime >> cutime >> cstime; // Grab active jiffies
      long totalTime = std::stol(utime) + std::stol(stime) + std::stol(cutime) + std::stol(cstime);
      return totalTime;
    }
  }
  return 0;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::string line;
  std::string key;
  CPUStates states{};
  long active_jiffies{0};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while(std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        linestream >> states.user >> states.nice >> states.system 
        >> states.idle >> states.iowait >> states.irq >> states.softirq 
        >> states.steal >> states.guest >> states.guestnice;
        active_jiffies = states.Active();
        return active_jiffies;
      }
    }
  }
  return active_jiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return Jiffies() - ActiveJiffies(); }

// Read and return CPU utilization
float LinuxParser::CpuUtilization() { return (float)ActiveJiffies() / (float)Jiffies(); }

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::string key = "processes";
  int processes = findValueByKey<int>(key, kStatFilename);
  return processes;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::string key = "procs_running";
  int processes = findValueByKey<int>(key, kStatFilename);
  return processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::string command = getValueOfFile<std::string>(std::to_string(pid) + kCmdlineFilename);
  return command;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  std::string key = "VmSize:";
  std::string ram = findValueByKey<std::string>(key, std::to_string(pid) + kStatusFilename);
  return std::to_string(std::stol(ram) / 1024);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::string key = "Uid:";
  std::string uid = findValueByKey<std::string>(key, std::to_string(pid) + kStatusFilename);
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  auto uid = Uid(pid);
  std::string line;
  std::string user;
  const std::string delimiter = ":";
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while(std::getline(filestream, line)) {
      size_t start = 0;
      std::vector<std::string> tokens;
      tokens.reserve(3);
      for (int i = 0; i < 3; ++i) {
        auto end = line.find(delimiter, start);
        tokens.push_back(line.substr(start, end - start));
        start = end + delimiter.size();
      }
      if (tokens[2] == uid) {
        user = tokens[0];
        return user;
      }
    } 
  }
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::string value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()){
    for (int i = 0; i < 22; ++i) filestream >> value;
      return LinuxParser::UpTime() - std::stol(value) / sysconf(_SC_CLK_TCK);
  }
  return 0;
}