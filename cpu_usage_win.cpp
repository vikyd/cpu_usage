// This program is for Windows only.
#include <stdio.h>
#include <Windows.h>
#include <iostream>

using namespace std;

typedef long long int64_t;
typedef unsigned long long uint64_t;

/// time convert
static uint64_t file_time_2_utc(const FILETIME *ftime)
{
  LARGE_INTEGER li;

  li.LowPart = ftime->dwLowDateTime;
  li.HighPart = ftime->dwHighDateTime;
  return li.QuadPart;
}

// get CPU num
static int get_processor_number()
{
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return (int)info.dwNumberOfProcessors;
}

int get_cpu_usage(int pid)
{
  static int processor_count_ = -1;
  static int64_t last_time_ = 0;
  static int64_t last_system_time_ = 0;

  FILETIME now;
  FILETIME creation_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;
  int64_t system_time;
  int64_t time;
  int64_t system_time_delta;
  int64_t time_delta;

  int cpu = -1;

  if (processor_count_ == -1)
  {
    processor_count_ = get_processor_number();
  }

  GetSystemTimeAsFileTime(&now);

  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
  if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
  {
    // can not find the process
    exit(EXIT_FAILURE);
  }
  system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / processor_count_;
  time = file_time_2_utc(&now);

  if ((last_system_time_ == 0) || (last_time_ == 0))
  {
    last_system_time_ = system_time;
    last_time_ = time;
    return get_cpu_usage(pid);
  }

  system_time_delta = system_time - last_system_time_;
  time_delta = time - last_time_;

  if (time_delta == 0)
  {
    return get_cpu_usage(pid);
  }

  cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
  last_system_time_ = system_time;
  last_time_ = time;
  return cpu;
}

// ref: http://www.cppblog.com/cppopp/archive/2012/08/24/188102.aspx
int main(int argc, char *argv[])
{
  int cpu;
  int process_id;
  // first arg is exeutable file
  if (argc <= 1)
  {
    // debug arguments
    // for (int i = 0; i < argc; i++)
    // {
    //   cout << "Argument " << i << " is " << argv[i] << endl;
    // }
  }
  process_id = atoi(argv[1]);

  cpu = get_cpu_usage(process_id);
  // sleep then get delta time of system and process, 100ms - 10s
  Sleep(500);
  cpu = get_cpu_usage(process_id);

  // print
  cout << cpu;

  exit(EXIT_SUCCESS);
}