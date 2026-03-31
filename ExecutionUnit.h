#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Basics.h"

class ExecutionUnit {
public:
    // per-unit reservation station
    UnitType name;
    int latency;
    std::vector<RSEntry> reservation_station;
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag
    int rs_size;
    std::vector<RSEntry*> pipeline;
    ExecutionUnit(UnitType name, int latency , int rs_size) {
        this -> name = name;
        this -> latency = latency;
        this -> rs_size = rs_size;
        reservation_station.resize(rs_size);
    }
    bool isfull();
    void capture(int tag, int val);
    void executeCycle();
private:
    void removeEntry();
    void pushIntoPipeline();
    void runInst(RSEntry* inst);
};