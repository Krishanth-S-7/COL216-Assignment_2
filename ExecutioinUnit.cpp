#include "ExecutionUnit.h"

void ExecutionUnit::capture(int tag, int val) {
    for (int i = 0; i < reservation_station.size(); i++) {
        if (!reservation_station[i].valid)
            continue;
        if (reservation_station[i].Tagj == tag) {
            reservation_station[i].Vj = true;
            reservation_station[i].Valuej = val;
        }
        if (reservation_station[i].Tagk == tag) {
            reservation_station[i].Vk = true;
            reservation_station[i].Valuek = val;
        }
        if (!reservation_station[i].working && !reservation_station[i].inQueue && reservation_station[i].Vj && reservation_station[i].Vk) {
            ready_inst.push(&reservation_station[i]);
            reservation_station[i].inQueue = true;
        }

    }
}

void ExecutionUnit::executeCycle() {
    has_result = false;
    has_exception = false;
    result_value = 0;
    if (pipeline.empty()) {
        pushIntoPipeline();
        if (!pipeline.empty() && pipeline.front()->current_latency == latency) {
            runInst(pipeline.front());
            result_tag = pipeline.front()->dest;
            removeEntry();
            pipeline.pop_front();
        }
        return;
    }
    for (auto& entry : pipeline)
        entry->current_latency++;
    if (pipeline.front()->current_latency == latency) {
        runInst(pipeline.front());
        result_tag = pipeline.front()->dest;
        removeEntry();
        pipeline.pop_front();
    }
    pushIntoPipeline();
}

void ExecutionUnit::pushIntoPipeline() {
    if (pipeline.size() == latency || ready_inst.empty()) return;
    pipeline.push_back(ready_inst.top());
    ready_inst.top()->working = true;
    ready_inst.top()->current_latency = 1;
    ready_inst.pop();
}

void ExecutionUnit::removeEntry() {
    if (pipeline.empty())
        return;
    reservation_station[pipeline.front()->ind].valid = false;
    reservation_station[pipeline.front()->ind].Vj = false;
    reservation_station[pipeline.front()->ind].Vk = false;
    reservation_station[pipeline.front()->ind].inQueue = false;
    reservation_station[pipeline.front()->ind].working = false;
    reservation_station[pipeline.front()->ind].current_latency = 0;
    available_ind.push(pipeline.front()->ind);
}


void adder(RSEntry* inst, int& result_value, bool& has_exception) {
    long long result = 0;
    if (inst->op == OpCode::ADD) {
        result = (long long)inst->Valuej + (long long)inst->Valuek;
    } else if (inst->op == OpCode::ADDI) {
        result = (long long)inst->Valuej + (long long)inst->Valuek;
    } else if (inst->op == OpCode::SUB) {
        result = (long long)inst->Valuej - (long long)inst->Valuek;
    } else if (inst->op == OpCode::SLT) {
        result = (long long)inst->Valuej < (long long)inst->Valuek;
    } else if (inst->op == OpCode::SLTI) {
        result = (long long)inst->Valuej < (long long)inst->Valuek;
    }
    if (result > INT_MAX || result < INT_MIN) {
        has_exception = true;
    } else {
        result_value = result;
    }
}
bool ExecutionUnit::isfull() { return available_ind.empty(); }

void multiplier(RSEntry* inst, int& result_value, bool& has_exception) {
    long long result = 0;
    if (inst->op == OpCode::MUL) {
        result = (long long)inst->Valuej * (long long)inst->Valuek;
    }
    if (result > INT_MAX || result < INT_MIN) {
        has_exception = true;
    } else {
        result_value = result;
    }
}

void divider(RSEntry* inst, int& result_value, bool& has_exception) {
    if (inst->Valuek == 0) {
        has_exception = true;
        return;
    }
    long long result = 0;
    if (inst->op == OpCode::DIV) {
        result = (long long)inst->Valuej / (long long)inst->Valuek;
    } else if (inst->op == OpCode::REM) {
        result = (long long)inst->Valuej % (long long)inst->Valuek;
    }
    if (result > INT_MAX || result < INT_MIN) {
        has_exception = true;
    } else {
        result_value = result;
    }
}

void logic(RSEntry* inst, int& result_value, bool& has_exception) {
    if (inst->op == OpCode::AND) {
        result_value = inst->Valuej & inst->Valuek;
    } else if (inst->op == OpCode::ANDI) {
        result_value = inst->Valuej & inst->Valuek;
    } else if (inst->op == OpCode::OR) {
        result_value = inst->Valuej | inst->Valuek;
    } else if (inst->op == OpCode::ORI) {
        result_value = inst->Valuej | inst->Valuek;
    } else if (inst->op == OpCode::XOR) {
        result_value = inst->Valuej ^ inst->Valuek;
    } else if (inst->op == OpCode::XORI) {
        result_value = inst->Valuej ^ inst->Valuek;
    }
}

void branch(RSEntry* inst, int& result_value, bool& has_exception) {
    if (inst->op == OpCode::BEQ) {
        result_value = (inst->Valuej == inst->Valuek);
    } else if (inst->op == OpCode::BNE) {
        result_value = (inst->Valuej != inst->Valuek);
    } else if (inst->op == OpCode::BLT) {
        result_value = (inst->Valuej < inst->Valuek);
    } else if (inst->op == OpCode::BLE) {
        result_value = (inst->Valuej <= inst->Valuek);
    }
}

void ExecutionUnit::runInst(RSEntry* inst) {
    if (name == UnitType::ADDER) {
        adder(inst, result_value, has_exception);
    } else if (name == UnitType::BRANCH) {
        branch(inst, result_value, has_exception);
    } else if (name == UnitType::DIVIDER) {
        divider(inst, result_value, has_exception);
    } else if (name == UnitType::LOGIC) {
        logic(inst, result_value, has_exception);
    } else if (name == UnitType::MULTIPLIER) {
        multiplier(inst, result_value, has_exception);
    }
    has_result = true;
}