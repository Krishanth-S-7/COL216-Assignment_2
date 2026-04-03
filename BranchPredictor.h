#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>

#include "Basics.h"

class BranchPredictor {
   public:
    int total_branches = 0;
    int correct_predictions = 0;
    int counter = 0;
    std::unordered_map<int, int> predictions;  // key is the instruction number and value is the state of predictor
                                               // during predicition of key instruction
    int predict(int current_pc, int imm, OpCode op) {
        if (op == OpCode::BEQ || op == OpCode::BNE || op == OpCode::BLT || op == OpCode::BLE) {
            if (counter >= 2) {
                return imm;
            } else {
                return current_pc + 1;
            }
        } else if (op == OpCode::J) {
            return imm;
        } else {
            return current_pc + 1;
        }
    }

    void update(int pc, int actual_target, bool taken, bool was_correct) {
        total_branches++;
        if (was_correct) {
            correct_predictions++;
        }
    }
};