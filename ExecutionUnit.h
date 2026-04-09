#pragma once
#include <deque>
#include <iostream>
#include <string>
#include <vector>

#include "Basics.h"


class ExecutionUnit {
   public:
    // per-unit reservation station
    UnitType name;
    int latency;
    std::vector<RSEntry> reservation_station;
    bool has_result = false;  // result flag
    int result_tag;
    int result_value;
    bool has_exception = false;  // exception flag
    int rs_size;
    long long inst_counts = 0;
    std::priority_queue<RSEntry*, std::vector<RSEntry*>, Compare> ready_inst;
    std::deque<RSEntry*> pipeline;
    int indToFree = -1;
    std::queue<int> available_ind;  // use this queue to get the currently available indexes to store it RSEntry
    ExecutionUnit(UnitType name, int latency, int rs_size) {
        this->name = name;
        this->latency = latency;
        this->rs_size = rs_size;
        reservation_station.resize(rs_size);
        for (int i = 0; i < rs_size; i++) available_ind.push(i);
    }
    bool isfull();
    void capture(int tag, int val);
    void executeCycle();

   private:
    void removeEntry();
    void pushIntoPipeline();
    void runInst(RSEntry* inst);
};