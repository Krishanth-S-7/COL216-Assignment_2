#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include "Basics.h"

class LoadStoreQueue {
public:
    // LSQ reservation station
    int latency;
    vector<RSEntry> lsq_queue;
    int start_index = 0;
    int end_index = 0;
    bool has_result = false; // result flag
    int rs_size;
    bool has_exception = false; // exception flag
    int store_data = 0;
    LoadStoreQueue( int latency, int rs_size) {
        this -> latency = latency;
        lsq_queue.resize(rs_size);
        this -> rs_size = rs_size;
    }
    bool isfull() {
        return (end_index + 1) % rs_size == start_index;
    }
    void capture(int tag, int val) {};
    void executeCycle(std::vector<int>& Memory) {};
};