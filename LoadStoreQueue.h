#pragma once
#include <iostream>
#include <deque>
#include <string>
#include <vector>
#include <map>

#include "Basics.h"

// struct Compare {
//     bool operator()(const RSEntry* a, const RSEntry* b) { return a->sequence_number > b->sequence_number; }
// };

class LoadStoreQueue {
   public:
    // LSQ reservation station
    int latency;
    std::vector<RSEntry> reservation_station;
    std::deque<RSEntry*> pipeline;
    std::priority_queue<RSEntry*, std::vector<RSEntry*>, Compare> ready_inst;
    std::map<int, std::pair<int, int>> uncommitedSw;
    std::queue<int> available_ind;  // use this queue to get the currently available indexes to store it RSEntry
    bool has_result = false;  // result flag
    int result_tag;
    int result_value;
    int result_value2;
    bool isSW = false;
    long long inst_counts = 0;
    long long lsq_inst = 0;
    int rs_size;
    bool has_exception = false;  // exception flag
    int store_data = 0;
    LoadStoreQueue(int latency, int rs_size) {
        this->latency = latency;
        this->rs_size = rs_size;
        reservation_station.resize(rs_size);
        for (int i = 0; i < rs_size; i++) available_ind.push(i);
    }
    bool isfull();
    void capture(int tag, int val);
    void executeCycle(std::vector<int>& Memory);
    private:
    void pushIntoPipeline();
    void removeEntry();
    void runInst(RSEntry* inst, std::vector<int>& Memory);
};