#include "Processor.h"

void Processor::broadcastOnCDB() {
    for (int i = 0; i < units.size(); i++) units[i].capture(broadcast_tag, broadcast_value);
    ROB[broadcast_tag].value = broadcast_value;
    ROB[broadcast_tag].ready = true;
}

void Processor::flush() {
    while (ROB[rob_head].valid) {
        ROB[rob_head].valid = false;
        rob_head++;
        rob_head %= ROB.size();
    }
    rob_head = 0;
    rob_tail = 0;
    for (int i = 0; i < units.size(); i++) {
        for (auto& entry : units[i].reservation_station) entry.valid = false;
        units[i].pipeline.clear();
        while (!units[i].ready_inst.empty()) units[i].ready_inst.pop();
        while (!units[i].available_ind.empty()) units[i].available_ind.pop();
        for (int j = 0; j < units[i].reservation_station.size(); j++) units[i].available_ind.push(j);
        units[i].has_exception = false;
        units[i].has_result = false;
        units[i].result_value = 0;
    }
    // will flush RAT and load store queues later
}

void Processor::stageExecuteAndBroadcast() {
    for (int i = 0; i < units.size(); i++) {
        units[i].executeCycle();
        if (units[i].has_exception) {
            ROB[units[i].result_tag].has_exception = true;
        }
        if (units[i].has_result) {
            broadcast_tag = units[i].result_tag;
            broadcast_value = units[i].result_value;
            broadcastOnCDB();
        }
    }
}

void Processor::stageCommit() {
    if (!ROB[rob_head].valid || !ROB[rob_head].ready) return;
    if (ROB[rob_head].has_exception) {
        exception = true;
        pc = ROB[rob_head].inst_number;
        flush();
        return;
    }
    if (ROB[rob_head].destReg == -1) { // this means it was a branch instruction's entry
        int state = bp.predictions[ROB[rob_head].inst_number];
        if (ROB[rob_head].value == 1) {
            if (state == 1) {
                bp.counter = 0;
            } else if (state == 2) {
                bp.counter = 1;
                flush();
                pc = ROB[rob_head].inst_number + inst_memory[ROB[rob_head].inst_number].imm;
            } else if (state == 3) {
                bp.counter = 2;
                flush();
                pc = ROB[rob_head].inst_number + inst_memory[ROB[rob_head].inst_number].imm;
            }
        } else {
            if (state == 0) {
                bp.counter = 1;
                flush();
                pc = ROB[rob_head].inst_number + 1;
            } else if (state == 1) {
                flush();
                pc = ROB[rob_head].inst_number + 1;
                bp.counter = 2;
            } else if (state == 2) {
                bp.counter = 3;
            }
        }
    } else if (ROB[rob_head].destReg != -2) {
        if (ROB[rob_head].destReg != 0) ARF[ROB[rob_head].destReg] = ROB[rob_head].value;
    }
    ROB[rob_head].valid = false;
    rob_head++;
    rob_head %= ROB.size();
    // need to update the RAT also, will do it later
}