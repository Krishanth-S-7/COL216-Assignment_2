#pragma once
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "Basics.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"
using namespace std;
class Processor {
    OpCode stringToOpCode(const std::string& str);
    Instruction instParser(const std::string& line, long long line_number);

   public:
    int pc;
    int clock_cycle;
    // pipeline registers

    std::vector<Instruction> inst_memory;
    Instruction fetch_reg;

    // architectural state (do not change)
    std::vector<int> ARF;     // regFile
    std::vector<int> Memory;  // Memory
    bool exception = false;   // exception bit
    std::vector<int> RAT;
    std::vector<ROBEntry> ROB;
    int rob_head = 0;
    int rob_tail = 0;
    int rob_count = 0;
    int broadcast_tag;
    int broadcast_value;

    // register alias table / reorder buffer

    std::vector<ExecutionUnit> units;
    LoadStoreQueue* lsq;
    BranchPredictor bp;
    bool is_stalled = false;
    Processor(ProcessorConfig& config) {
        pc = 0;
        clock_cycle = 0;
        ARF.resize(config.num_regs, 0);
        Memory.resize(config.mem_size);
        ROB.resize(config.rob_size);
        RAT.resize(config.num_regs, -1);
        // Instantiate Hardware Units
        ExecutionUnit adder(UnitType::ADDER, config.add_lat, config.adder_rs_size);
        ExecutionUnit multiplier(UnitType::MULTIPLIER, config.mul_lat, config.mult_rs_size);
        ExecutionUnit divider(UnitType::DIVIDER, config.div_lat, config.div_rs_size);
        ExecutionUnit branch(UnitType::BRANCH, config.add_lat, config.br_rs_size);
        ExecutionUnit logic(UnitType::LOGIC, config.logic_lat, config.logic_rs_size);

        units.push_back(adder);
        units.push_back(multiplier);
        units.push_back(divider);
        units.push_back(branch);
        units.push_back(logic);

        // Adder
        // Multiplier
        // Divider
        // Branch Computation
        // Bitwise Logic
        // Load-Store Unit
    }
    void loadProgram(const std::string& filename);

    void flush();

    void broadcastOnCDB();
    void stageFetch();
    void stageDecode();
    void stageExecuteAndBroadcast();
    void InitializeROBEntry(bool valid, bool ready, int destReg, int value, int inst_number);
    void setUpRSEntry(int src_reg, int& value, int& tag, bool& is_ready);
    void stageCommit();

    bool step() {
        clock_cycle++;
        return true;
    }

    void dumpArchitecturalState() {
        std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
        for (int i = 0; i < ARF.size(); i++) {
            std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
            if ((i + 1) % 8 == 0)
                std::cout << std::endl;
        }
        if (exception) {
            std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
        }
        std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/" << bp.total_branches << " correct.\n";
    }
};