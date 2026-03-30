#pragma once
#include "Basics.h"
#include <iostream>
#include <vector>

class BranchPredictor {
public:
    int total_branches = 0;
    int correct_predictions = 0;
    int counter = 0;
    int predict(int current_pc, int imm, OpCode op) {
        if(counter >= 2) {
            return current_pc + imm; 
        }
        else {
            return current_pc + 1;
        }
        // return current_pc + imm; 
    }

    void update(int pc, int actual_target, bool taken, bool was_correct) {
        total_branches++;
        if (was_correct) {
            correct_predictions++;
        }
    }
};