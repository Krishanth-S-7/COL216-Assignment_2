#include "ExecutionUnit.h"

void ExecutionUnit::capture(int tag, int val) {
    for (int i = 0; i < reservation_station.size(); i++) {
        if (reservation_station[i].Tagj == tag) reservation_station[i].Valuej = val;
        if (reservation_station[i].Tagk == tag) reservation_station[i].Valuek = val;
    }
}

void ExecutionUnit::executeCycle() {
    ReservationStationEntry* inst = nullptr;
    for (int i = 0; i < reservation_station.size(); i++) {
        if (reservation_station[i].Vj && reservation_station[i].Vk) {
            inst = &reservation_station[i];
            break;
        }
    }
    if (!inst) return;
    inst->working = true;
    int result;
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

void adder(ReservationStationEntry* inst) {
    if (inst->op == OpCode::ADD) {

    } else if (inst->op == OpCode::ADDI) {

    } else if (inst->op == OpCode::SUB) {

    } else if (inst->op == OpCode::SLT) {

    } else if (inst->op == OpCode::SLTI) {

    }
 }

void multiplier(ReservationStationEntry* inst) {
    if (inst->op == OpCode::MUL) {

    }
}

void divider(ReservationStationEntry* inst) {
    if (inst->op == OpCode::DIV) {

    } else if (inst->op == OpCode::REM) {

    }
}

void logic(ReservationStationEntry* inst) {
    if (inst->op == OpCode::AND) {

    } else if (inst->op == OpCode::ANDI) {

    } else if (inst->op == OpCode::OR) {

    } else if (inst->op == OpCode::ORI) {

    } else if (inst->op == OpCode::XOR) {

    } else if (inst->op == OpCode::XORI) {

    }
}

void branch(ReservationStationEntry* inst) {
    if (inst->op == OpCode::BEQ) {

    } else if (inst->op == OpCode::BNE) {

    } else if (inst->op == OpCode::BLT) {

    } else if (inst->op == OpCode::BLE) {

    }
}

void loadstore(ReservationStationEntry* inst) {
    if (inst->op == OpCode::LW) {

    } else if (inst->op == OpCode::SW) {
        
    }
}