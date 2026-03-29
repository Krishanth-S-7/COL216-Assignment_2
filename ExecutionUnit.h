#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Basics.h"

struct ReservationStationEntry {
    bool working = false;
    OpCode op;
    int Vj;
    int Vk;
    int Tagj;
    int Tagk;
    int Valuej;
    int Valuek;
    int dest;
    int current_latency;
};
class ExecutionUnit {
public:
    // per-unit reservation station
    UnitType name;
    int latency;
    std::vector<ReservationStationEntry> reservation_station;
    bool has_result = false; // result flag
    bool has_exception = false; // exception flag
    int rs_size;
    ExecutionUnit(UnitType name, int latency , int rs_size) {
        this -> name = name;
        this -> latency = latency;
        this -> rs_size = rs_size;
    }
    void capture(int tag, int val) {};
    void executeCycle() {};
};