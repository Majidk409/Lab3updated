#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <climits>

using namespace std;

struct Process {
    // Basic process information
    int arrivalTime;                // Arrival time of the process
    int remainingTime;
    // Burst times
    std::vector<int> cpuBursts;     // List of CPU burst times
    std::vector<int> ioBursts;      // List of I/O burst times
    
    // Current burst management
    int currentCpuBurst = 0;        // Index for the current CPU burst
    int currentIoBurst = 0;         // Index for the current I/O burst
    
    // Time metrics for scheduling
    int remainingCpuTime = 0;       // Remaining CPU time for SJF and SRTF
    int turnaroundTime = 0;         // Turnaround time of the process
    int waitingTime = 0;            // Waiting time of the process
    int completionTime = 0;         // Completion time of the process
    int totalCpuBurstTime = 0;      // Total CPU burst time of the process
    
    // Status flags
    bool completed = false;         // Flag to indicate if the process is completed
    bool inIO = false;              // Flag to indicate if the process is in I/O state
    
    // Additional timing information
    int ioCompletionTime = 0;       // Time when the I/O operation will complete
    
    // Indices for managing burst queues
    int currentCpuBurstIndex = 0;   // Index for the current CPU burst
    int currentIoBurstIndex = 0;    // Index for the current I/O burst
};

// Read workload file
vector<Process> readWorkloadFile(const string& filePath) {
    ifstream infile(filePath);
    vector<Process> processes;
    string line;

    while (getline(infile, line)) {
        Process p;
        istringstream iss(line);
        int value;

        iss >> p.arrivalTime;

        bool isCpuBurst = true;
        while (iss >> value && value != -1) {
            if (isCpuBurst) {
                p.cpuBursts.push_back(value);
            } else {
                p.ioBursts.push_back(value);
            }
            isCpuBurst = !isCpuBurst;
        }

        p.remainingTime = accumulate(p.cpuBursts.begin(), p.cpuBursts.end(), 0);
        processes.push_back(p);
    }

    return processes;
}

// FIFO Scheduling
void fifoScheduling(vector<Process>& processes) {
    int currentTime = 0;
    vector<int> readyQueue;
    int numProcesses = processes.size();
    int processesCompleted = 0;

    while (processesCompleted < numProcesses) {
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].inIO && !processes[i].completed) {
                if (find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                    readyQueue.push_back(i);
                }
            }
        }

        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].inIO && processes[i].ioCompletionTime <= currentTime) {
                readyQueue.push_back(i);
                processes[i].inIO = false;
            }
        }

        if (!readyQueue.empty()) {
            int processID = readyQueue.front();
            readyQueue.erase(readyQueue.begin());

            Process& process = processes[processID];
            cout << "Executing Process " << processID + 1 << ", CPU Burst " << process.currentCpuBurst + 1 << endl;
            currentTime += process.cpuBursts[process.currentCpuBurst];
            process.currentCpuBurst++;

            if (process.currentCpuBurst < process.cpuBursts.size()) {
                process.inIO = true;
                process.ioCompletionTime = currentTime + process.ioBursts[process.currentIoBurst++];
            } else {
                process.completionTime = currentTime;
                process.completed = true;
                processesCompleted++;
            }
        } else {
            int nextEventTime = INT_MAX;
            for (int i = 0; i < numProcesses; i++) {
                if (!processes[i].completed) {
                    if (processes[i].arrivalTime > currentTime) {
                        nextEventTime = min(nextEventTime, processes[i].arrivalTime);
                    }
                    if (processes[i].inIO) {
                        nextEventTime = min(nextEventTime, processes[i].ioCompletionTime);
                    }
                }
            }

            if (nextEventTime != INT_MAX) {
                currentTime = nextEventTime;
                cout << "No process ready at time " << currentTime << ". Advancing time." << endl;
            }
        }
    }

    for (auto& process : processes) {
        process.totalCpuBurstTime = accumulate(process.cpuBursts.begin(), process.cpuBursts.end(), 0);
        process.turnaroundTime = process.completionTime - process.arrivalTime;
        process.waitingTime = process.turnaroundTime - process.totalCpuBurstTime;
    }
}

// Shortest Job First (SJF)
void sjfScheduling(vector<Process>& processes) {
    int currentTime = 0;
    vector<int> readyQueue;
    int numProcesses = processes.size();
    int processesCompleted = 0;

    while (processesCompleted < numProcesses) {
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].inIO && !processes[i].completed) {
                if (find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                    readyQueue.push_back(i);
                }
            }
        }

        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].inIO && processes[i].ioCompletionTime <= currentTime) {
                readyQueue.push_back(i);
                processes[i].inIO = false;
            }
        }

        if (!readyQueue.empty()) {
            int shortestJobID = readyQueue[0];
            for (int id : readyQueue) {
                if (processes[id].cpuBursts[processes[id].currentCpuBurst] < processes[shortestJobID].cpuBursts[processes[shortestJobID].currentCpuBurst]) {
                    shortestJobID = id;
                }
            }

            readyQueue.erase(remove(readyQueue.begin(), readyQueue.end(), shortestJobID), readyQueue.end());

            Process& process = processes[shortestJobID];
            cout << "Executing Process " << shortestJobID + 1 << ", CPU Burst " << process.currentCpuBurst + 1 << endl;
            currentTime += process.cpuBursts[process.currentCpuBurst];
            process.currentCpuBurst++;

            if (process.currentCpuBurst < process.cpuBursts.size()) {
                process.inIO = true;
                process.ioCompletionTime = currentTime + process.ioBursts[process.currentIoBurst++];
            } else {
                process.completionTime = currentTime;
                process.completed = true;
                processesCompleted++;
            }
        } else {
            int nextEventTime = INT_MAX;
            for (int i = 0; i < numProcesses; i++) {
                if (!processes[i].completed) {
                    if (processes[i].arrivalTime > currentTime) {
                        nextEventTime = min(nextEventTime, processes[i].arrivalTime);
                    }
                    if (processes[i].inIO) {
                        nextEventTime = min(nextEventTime, processes[i].ioCompletionTime);
                    }
                }
            }

            if (nextEventTime != INT_MAX) {
                currentTime = nextEventTime;
                cout << "No process ready at time " << currentTime << ". Advancing time." << endl;
            }
        }
    }

    for (auto& process : processes) {
        process.totalCpuBurstTime = accumulate(process.cpuBursts.begin(), process.cpuBursts.end(), 0);
        process.turnaroundTime = process.completionTime - process.arrivalTime;
        process.waitingTime = process.turnaroundTime - process.totalCpuBurstTime;
    }
}

// Shortest Remaining Time First (SRTF)
void srtfScheduling(vector<Process>& processes) {
    int currentTime = 0;
    vector<int> readyQueue;
    int numProcesses = processes.size();
    vector<int> remainingTime(numProcesses, 0);
    int processesCompleted = 0;

    for (int i = 0; i < numProcesses; i++) {
        if (!processes[i].cpuBursts.empty()) {
            remainingTime[i] = processes[i].cpuBursts[processes[i].currentCpuBurst];
        }
    }

    while (processesCompleted < numProcesses) {
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].inIO && !processes[i].completed) {
                if (find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                    readyQueue.push_back(i);
                }
            }
        }

        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].inIO && processes[i].ioCompletionTime <= currentTime) {
                processes[i].inIO = false;
                if (find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                    readyQueue.push_back(i);
                }
                if (processes[i].currentCpuBurst < processes[i].cpuBursts.size()) {
                    remainingTime[i] = processes[i].cpuBursts[processes[i].currentCpuBurst];
                }
            }
        }

        if (!readyQueue.empty()) {
            int shortestTimeID = readyQueue[0];
            for (int id : readyQueue) {
                if (remainingTime[id] < remainingTime[shortestTimeID]) {
                    shortestTimeID = id;
                }
            }

            Process& process = processes[shortestTimeID];
            int timeSlice = min(remainingTime[shortestTimeID], 1);

            cout << "Executing Process " << shortestTimeID + 1 << " for " << timeSlice << " unit(s)" << endl;
            currentTime += timeSlice;
            remainingTime[shortestTimeID] -= timeSlice;

            if (remainingTime[shortestTimeID] == 0) {
                process.currentCpuBurst++;
                if (process.currentCpuBurst < process.cpuBursts.size()) {
                    process.inIO = true;
                    process.ioCompletionTime = currentTime + process.ioBursts[process.currentIoBurst++];
                    remainingTime[shortestTimeID] = 0;  // No more remaining CPU time for this burst
                } else {
                    process.completionTime = currentTime;
                    process.completed = true;
                    processesCompleted++;
                    remainingTime[shortestTimeID] = 0;  // Mark as complete
                }
                readyQueue.erase(remove(readyQueue.begin(), readyQueue.end(), shortestTimeID), readyQueue.end());
            }
        } else {
            int nextEventTime = INT_MAX;
            for (int i = 0; i < numProcesses; i++) {
                if (!processes[i].completed) {
                    if (processes[i].arrivalTime > currentTime) {
                        nextEventTime = min(nextEventTime, processes[i].arrivalTime);
                    }
                    if (processes[i].inIO) {
                        nextEventTime = min(nextEventTime, processes[i].ioCompletionTime);
                    }
                }
            }

            if (nextEventTime != INT_MAX) {
                currentTime = nextEventTime;
                cout << "No process ready at time " << currentTime << ". Advancing time." << endl;
            }
        }
    }

    for (auto& process : processes) {
        process.totalCpuBurstTime = accumulate(process.cpuBursts.begin(), process.cpuBursts.end(), 0);
        process.turnaroundTime = process.completionTime - process.arrivalTime;
        process.waitingTime = process.turnaroundTime - process.totalCpuBurstTime;
    }
}

// Completely Fair Scheduler (CFS)
void cfsScheduling(vector<Process>& processes) {
    int currentTime = 0;
    vector<int> readyQueue;
    int numProcesses = processes.size();
    int processesCompleted = 0;
    const int timeSlice = 1; // CFS time slice

    while (processesCompleted < numProcesses) {
        // Add new arrivals to the ready queue
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].inIO && !processes[i].completed) {
                if (find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                    readyQueue.push_back(i);
                }
            }
        }

        // Check if any process has completed its I/O and is ready for the next CPU burst
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].inIO && processes[i].ioCompletionTime <= currentTime) {
                readyQueue.push_back(i);
                processes[i].inIO = false;
            }
        }

        if (!readyQueue.empty()) {
            int processID = readyQueue.front();
            readyQueue.erase(readyQueue.begin());

            Process& process = processes[processID];
            int burstTime = process.cpuBursts[process.currentCpuBurst];
            int slice = min(burstTime, timeSlice);

            cout << "Executing Process " << processID + 1 << ", CPU Burst " << process.currentCpuBurst + 1 << " for " << slice << " units\n";
            currentTime += slice;
            process.cpuBursts[process.currentCpuBurst] -= slice;

            if (process.cpuBursts[process.currentCpuBurst] == 0) {
                process.currentCpuBurst++;
                if (process.currentCpuBurst < process.cpuBursts.size()) {
                    process.inIO = true;
                    process.ioCompletionTime = currentTime + process.ioBursts[process.currentIoBurst++];
                } else {
                    process.completionTime = currentTime;
                    process.completed = true;
                    processesCompleted++;
                }
            } else {
                readyQueue.push_back(processID);
            }
        } else {
            int nextEventTime = INT_MAX;
            for (int i = 0; i < numProcesses; i++) {
                if (!processes[i].completed) {
                    if (processes[i].arrivalTime > currentTime) {
                        nextEventTime = min(nextEventTime, processes[i].arrivalTime);
                    }
                    if (processes[i].inIO) {
                        nextEventTime = min(nextEventTime, processes[i].ioCompletionTime);
                    }
                }
            }

            if (nextEventTime != INT_MAX) {
                currentTime = nextEventTime;
                cout << "No process ready at time " << currentTime << ". Advancing time." << endl;
            }
        }
    }

    for (auto& process : processes) {
        process.totalCpuBurstTime = accumulate(process.cpuBursts.begin(), process.cpuBursts.end(), 0);
        process.turnaroundTime = process.completionTime - process.arrivalTime;
        process.waitingTime = process.turnaroundTime - process.totalCpuBurstTime;
    }
}


// Round Robin Scheduling
void roundRobinScheduling(vector<Process>& processes, int quantum) {
    int currentTime = 0;
    deque<int> readyQueue;
    int numProcesses = processes.size();
    int processesCompleted = 0;

    // Initialize remainingCpuTime for each process
    for (int i = 0; i < numProcesses; i++) {
        if (!processes[i].cpuBursts.empty()) {
            processes[i].remainingCpuTime = processes[i].cpuBursts[0];
        }
    }

    while (processesCompleted < numProcesses) {
        // Add processes that have arrived to the ready queue
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].inIO && !processes[i].completed) {
                if (find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                    readyQueue.push_back(i);
                }
            }
        }

        // Handle IO completions
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].inIO && processes[i].ioCompletionTime <= currentTime) {
                processes[i].inIO = false;
                if (find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                    readyQueue.push_back(i);
                }
                if (processes[i].currentCpuBurstIndex < processes[i].cpuBursts.size()) {
                    processes[i].remainingCpuTime = processes[i].cpuBursts[processes[i].currentCpuBurstIndex];
                }
            }
        }

        if (!readyQueue.empty()) {
            int processID = readyQueue.front();
            readyQueue.pop_front();
            Process& process = processes[processID];
            int timeSlice = min(process.remainingCpuTime, quantum);

            cout << "Executing Process " << processID + 1 << " for " << timeSlice << " unit(s)" << endl;

            // Update time and remaining CPU time
            currentTime += timeSlice;
            process.remainingCpuTime -= timeSlice;

            // Check if the process has finished its CPU burst
            if (process.remainingCpuTime == 0) {
                process.currentCpuBurstIndex++;
                if (process.currentCpuBurstIndex < process.cpuBursts.size()) {
                    process.inIO = true;
                    process.ioCompletionTime = currentTime + process.ioBursts[process.currentIoBurstIndex++];
                    process.remainingCpuTime = 0; // Reset remaining CPU time during I/O
                } else {
                    process.completionTime = currentTime;
                    process.completed = true;
                    processesCompleted++;
                }
            } else {
                readyQueue.push_back(processID);
            }
        } else {
            // Advance time to the next event if no process is ready
            int nextEventTime = INT_MAX;
            for (int i = 0; i < numProcesses; i++) {
                if (!processes[i].completed) {
                    if (processes[i].arrivalTime > currentTime) {
                        nextEventTime = min(nextEventTime, processes[i].arrivalTime);
                    }
                    if (processes[i].inIO) {
                        nextEventTime = min(nextEventTime, processes[i].ioCompletionTime);
                    }
                }
            }

            if (nextEventTime != INT_MAX) {
                currentTime = nextEventTime;
                cout << "No process ready at time " << currentTime << ". Advancing time." << endl;
            }
        }
    }

    // Calculate turnaround time and waiting time
    for (auto& process : processes) {
        int totalCpuBurstTime = accumulate(process.cpuBursts.begin(), process.cpuBursts.end(), 0);
        process.turnaroundTime = process.completionTime - process.arrivalTime;
        process.waitingTime = process.turnaroundTime - totalCpuBurstTime;
    }

    // Print process metrics
    cout << "Process Arrival Time    TotalCpuBurst   Completion Time TAT              WT" << endl;
    for (int i = 0; i < numProcesses; i++) {
        cout << "P" << i + 1 << "      "
             << processes[i].arrivalTime <<"\t\t"
             << accumulate(processes[i].cpuBursts.begin(), processes[i].cpuBursts.end(), 0) << "\t\t"
             << processes[i].completionTime << "\t\t"
             << processes[i].turnaroundTime << "\t\t"
             << processes[i].waitingTime << endl;
    }

    // Calculate and print average turnaround time and waiting time
    double totalTAT = accumulate(processes.begin(), processes.end(), 0.0,
        [](double sum, const Process& p) { return sum + p.turnaroundTime; });
    double totalWT = accumulate(processes.begin(), processes.end(), 0.0,
        [](double sum, const Process& p) { return sum + p.waitingTime; });

    cout << "Average Turnaround Time (ATAT): " << totalTAT / numProcesses << endl;
    cout << "Average Waiting Time (AWT): " << totalWT / numProcesses << endl;
}
void calculateAndPrintMetrics(const vector<Process>& processes) {
    double totalTAT = 0, totalWT = 0;
    int numProcesses = processes.size();

    cout << "\nProcess\tArrival Time\tTotalCpuBurst\tCompletion Time\tTAT\tWT\n";
    for (const auto& process : processes) {
        cout << "P" << (&process - &processes[0]) + 1 << "\t"
             << process.arrivalTime << "\t\t"
             << process.totalCpuBurstTime <<"\t\t"
             << process.completionTime << "\t\t"
             << process.turnaroundTime << "\t"
             << process.waitingTime << "\n";
            

        totalTAT += process.turnaroundTime;
        totalWT += process.waitingTime;
    }

    double averageTAT = totalTAT / numProcesses;
    double averageWT = totalWT / numProcesses;

    cout << "\nAverage Turnaround Time (ATAT): " << averageTAT << endl;
    cout << "Average Waiting Time (AWT): " << averageWT << endl;
}

// Main function
int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file> [<Time Quantum>]" << endl;
        return 1;
    }

    string schedulingAlgorithm = argv[1];
    string filePath = argv[2];

    int tq = 0;
    if (schedulingAlgorithm == "RR") {
        if (argc != 4) {
            cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file> <Time Quantum>" << endl;
            return 1;
        }
        tq = stoi(argv[3]);  // Convert the third argument to an integer for the time quantum
    } else {
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file>" << endl;
            return 1;
        }
    }

    vector<Process> processes = readWorkloadFile(filePath);

    if (schedulingAlgorithm == "FIFO") {
        fifoScheduling(processes);
    } else if (schedulingAlgorithm == "SJF") {
        sjfScheduling(processes);
    } else if (schedulingAlgorithm == "SRTF") {
        srtfScheduling(processes);
    } else if (schedulingAlgorithm == "CFS") {
        cfsScheduling(processes);
    } else if (schedulingAlgorithm == "RR") {
        roundRobinScheduling(processes, tq);
    } else {
        cerr << "Unsupported scheduling algorithm!" << endl;
        return 1;
    }
#ifdef tq
    calculateAndPrintMetrics(processes);
#endif
    return 0;
}
