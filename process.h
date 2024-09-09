#ifndef PROCESS_H
#define PROCESS_H

#include <vector>

class Process {
public:
    int id;
    int arrival_time;
    std::vector<int> cpu_bursts;
    std::vector<int> io_bursts;
    int current_burst;
    int remaining_time;
    int turnaround_time;
    int waiting_time;
    int completion_time;
    int response_time;
    int start_time;

    Process(int id, int arrival_time, const std::vector<int>& cpu_bursts, const std::vector<int>& io_bursts)
        : id(id), arrival_time(arrival_time), cpu_bursts(cpu_bursts), io_bursts(io_bursts), current_burst(0),
          remaining_time(cpu_bursts.empty() ? 0 : cpu_bursts[0]), turnaround_time(0), waiting_time(0), 
          completion_time(0), response_time(-1), start_time(-1) {}
};

#endif // PROCESS_H
