#include "ExecutionUnit.h"

void ExecutionUnit::capture(int tag, int val) {
    for (int i = 0; i < reservation_station.size(); i++) {
        if (reservation_station[i].Tagj == tag) reservation_station[i].Valuej = val;
        if (reservation_station[i].Tagk == tag) reservation_station[i].Valuek = val;
    }
}

void ExecutionUnit::executeCycle() {
    if (pipeline.empty()) {
        pushIntoPipeline();
        if (!pipeline.empty() && pipeline.front()->current_latency == latency) {
            runInst(pipeline[0]);
            removeEntry();
            pipeline.pop_back();
        }
        return;
    }
    for (RSEntry* entry : pipeline) entry->current_latency++;
    if (pipeline[0]->current_latency == latency) {
        runInst(pipeline[0]);
        removeEntry();
        pipeline.erase(pipeline.begin());
    }
    pushIntoPipeline();
}

void ExecutionUnit::pushIntoPipeline() {
    for (int i = 0; i < reservation_station.size(); i++) {
        if (reservation_station[i].Vj && reservation_station[i].Vk) {
            reservation_station[i].current_latency++;
            pipeline.push_back(&reservation_station[i]);
            return;
        }
    }
}

void ExecutionUnit::removeEntry() {
    if (pipeline.empty()) return;
    int ind = 0;
    while (&reservation_station[ind] != pipeline[0]) ind++;
    reservation_station.erase(reservation_station.begin()+ind);
}

void ExecutionUnit::runInst(RSEntry* inst) {
    if (name == UnitType::ADDER) {
        adder(inst);
    } else if (name == UnitType::BRANCH) {
        branch(inst);
    } else if (name == UnitType::DIVIDER) {
        divider(inst);
    } else if (name == UnitType::LOADSTORE) {
        loadstore(inst);
    } else if (name == UnitType::LOGIC) {
        logic(inst);
    } else if (name == UnitType::MULTIPLIER) {
        multiplier(inst);
    }
}

void adder(RSEntry* inst) {
    if (inst->op == OpCode::ADD) {

    } else if (inst->op == OpCode::ADDI) {

    } else if (inst->op == OpCode::SUB) {

    } else if (inst->op == OpCode::SLT) {

    } else if (inst->op == OpCode::SLTI) {

    }
 }
 bool ExecutionUnit::isfull(){
    for (auto& entry : reservation_station) {
        if (!entry.valid) return false;
    }
    return true;
}

void multiplier(RSEntry* inst) {
    if (inst->op == OpCode::MUL) {

    }
}

void divider(RSEntry* inst) {
    if (inst->op == OpCode::DIV) {

    } else if (inst->op == OpCode::REM) {

    }
}

void logic(RSEntry* inst) {
    if (inst->op == OpCode::AND) {

    } else if (inst->op == OpCode::ANDI) {

    } else if (inst->op == OpCode::OR) {

    } else if (inst->op == OpCode::ORI) {

    } else if (inst->op == OpCode::XOR) {

    } else if (inst->op == OpCode::XORI) {

    }
}

void branch(RSEntry* inst) {
    if (inst->op == OpCode::BEQ) {

    } else if (inst->op == OpCode::BNE) {

    } else if (inst->op == OpCode::BLT) {

    } else if (inst->op == OpCode::BLE) {

    }
}

void loadstore(RSEntry* inst) {
    if (inst->op == OpCode::LW) {

    } else if (inst->op == OpCode::SW) {
        
    }
}