#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>

#include "Basics.h"

class BranchPredictor {
   public:
    int total_branches = 0;
    int correct_predictions = 0;
    // int counter = 0;

    vector<int> instruction_state;
    vector<queue<int>> predictionslist;
    BranchPredictor() {
        instruction_state.resize(1024, 0);
        predictionslist.resize(1024);
    }
    int predict(int current_pc, int imm, OpCode op) {
        if (op == OpCode::BEQ || op == OpCode::BNE || op == OpCode::BLT || op == OpCode::BLE) {
            int counter = instruction_state[current_pc];
            if (counter >= 2) {
                return current_pc + 1;
            } else {
                return imm;
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