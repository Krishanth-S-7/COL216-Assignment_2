#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "Basics.h"
#include "BranchPredictor.h"
#include "ExecutionUnit.h"
#include "LoadStoreQueue.h"
using namespace std;
class Processor {
    OpCode stringToOpCode(const std::string& str) {
        if (str == "add") return OpCode::ADD;
        if (str == "sub") return OpCode::SUB;
        if (str == "addi") return OpCode::ADDI;
        if (str == "mul") return OpCode::MUL;
        if (str == "div") return OpCode::DIV;
        if (str == "rem") return OpCode::REM;
        if (str == "lw") return OpCode::LW;
        if (str == "sw") return OpCode::SW;
        if (str == "beq") return OpCode::BEQ;
        if (str == "bne") return OpCode::BNE;
        if (str == "blt") return OpCode::BLT;
        if (str == "ble") return OpCode::BLE;
        if (str == "j") return OpCode::J;
        if (str == "slt") return OpCode::SLT;
        if (str == "slti") return OpCode::SLTI;
        if (str == "and") return OpCode::AND;
        if (str == "or") return OpCode::OR;
        if (str == "xor") return OpCode::XOR;
        if (str == "andi") return OpCode::ANDI;
        if (str == "ori") return OpCode::ORI;
        if (str == "xori") return OpCode::XORI;
        throw std::invalid_argument("Invalid opcode: " + str);
    }
    Instruction instParser(const std::string& line , long long line_number) {
        Instruction inst;
        string changed_line = line;
        replace(changed_line.begin(), changed_line.end(), ',', ' ');
        replace(changed_line.begin(), changed_line.end(), '(', ' ');
        replace(changed_line.begin(), changed_line.end(), ')', ' ');  

        std::istringstream iss(changed_line);
        string word;
        iss >> word;
        inst.op = stringToOpCode(word);
        if(word == "add" || word =="sub" || word == "mul" || word == "div" || word == "rem" || word == "and" || word == "or" || word == "xor" || word == "slt"){
            string rd, rs1, rs2;
            iss >> rd >> rs1 >> rs2;
            inst.dest = stoi(rd.substr(1));
            inst.src1 = stoi(rs1.substr(1));
            inst.src2 = stoi(rs2.substr(1));
        }
        else if(word == "addi" || word == "slti" || word == "andi" || word == "ori" || word == "xori"){
            string rd, rs1, imm;
            iss >> rd >> rs1 >> imm;
            inst.dest = stoi(rd.substr(1));
            inst.src1 = stoi(rs1.substr(1));
            inst.imm = stoi(imm);
        }
        else if (word == "lw"){
            string rd, offset, rs1;
            iss >> rd >> offset >> rs1;
            inst.dest = stoi(rd.substr(1));
            inst.src1 = stoi(rs1.substr(1));
            inst.imm = stoi(offset);
        }
        else if(word == "sw"){
            string rs2, offset, rs1;
            iss >> rs2 >> offset >> rs1;
            inst.src2 = stoi(rs2.substr(1));
            inst.src1 = stoi(rs1.substr(1));
            inst.imm = stoi(offset);
        }
        else if(word == "beq" || word == "bne" || word == "blt" || word == "ble"){
            string rs1, rs2, imm;
            iss >> rs1 >> rs2 >> imm;
            inst.src1 = stoi(rs1.substr(1));
            inst.src2 = stoi(rs2.substr(1));
            inst.imm = stoi(imm);
        }
        else if(word == "j"){
            string imm;
            iss >> imm;
            inst.imm = stoi(imm);
        }
        inst.pc = line_number;
        return inst;
    }
public:
    int pc;
    int clock_cycle;

    // pipeline registers

    std::vector<Instruction> inst_memory;
    Instruction fetch_reg;

    // architectural state (do not change)
    std::vector<int> ARF; // regFile
    std::vector<int> Memory; // Memory
    bool exception = false; // exception bit
    std::vector<int> RAT;
    std::vector<ROBEntry> ROB;
    int rob_head = 0;
    int rob_tail = 0;
    int rob_count = 0; 

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
        ExecutionUnit adder(UnitType::ADDER, config.add_lat , config.adder_rs_size);
        ExecutionUnit multiplier(UnitType::MULTIPLIER, config.mul_lat , config.mult_rs_size);
        ExecutionUnit divider(UnitType::DIVIDER, config.div_lat , config.div_rs_size);
        ExecutionUnit branch(UnitType::BRANCH, config.logic_lat , config.br_rs_size);
        ExecutionUnit logic(UnitType::LOGIC, config.logic_lat , config.logic_rs_size);

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
    void loadProgram(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }
        for(int i = 0 ; i < ARF.size(); i++) ARF[i] = 0;
        for(int i = 0 ; i < Memory.size(); i++) Memory[i] = 0;
        long long current_memory_index = 0;
        std::string line;
        long long line_number = 0;
        while (std::getline(file, line)) {
            if(line.empty()) continue;
            size_t start_index = (line[0] == '-') ? 1 : 0;
            if(line.length() <= start_index) continue;
            if( line.find_first_not_of("0123456789", start_index) == string::npos){
                this -> Memory[current_memory_index++] = stoi(line);
            }
            else{
                this -> inst_memory.push_back(instParser(line , line_number));
                line_number++;
            }
        }

    }

    void flush();

    void broadcastOnCDB();

    void stageFetch() {
        if (is_stalled) return;
        if (pc < inst_memory.size()) {
            fetch_reg = inst_memory[pc];
            pc = bp.predict(pc, fetch_reg.imm, fetch_reg.op);
        }else{
            fetch_reg.pc = -1;
        }
    };

    void stageDecode() {
        if(fetch_reg.pc == -1) return;
        if (rob_count == ROB.size()) {
            is_stalled = true;
            return;
        }
        if (fetch_reg.op == OpCode::ADD || fetch_reg.op == OpCode::SUB || fetch_reg.op == OpCode::ADDI) {
            if(units[0].isfull()) {
                is_stalled = true;
                return;
            }
            ROB[rob_tail].valid = true;
            ROB[rob_tail].ready = false;
            ROB[rob_tail].destReg = fetch_reg.dest;
            ROB[rob_tail].value = -1;
            RSEntry entry;
            entry.valid = true;
            entry.op = fetch_reg.op;    
            if(RAT[fetch_reg.src1] != -1) {
                int rob_idx = RAT[fetch_reg.src1];
                if (ROB[rob_idx].ready) {
                    entry.Valuej = ROB[rob_idx].value;
                    entry.Vj = true;
                } else {
                    entry.Tagj = rob_idx;
                    entry.Vj = false;
                }
            } else {
                entry.Valuej = ARF[fetch_reg.src1];
                entry.Vj = true;
            }
            if(fetch_reg.op == OpCode::ADDI){
                entry.Valuek = fetch_reg.imm;
                entry.Vk = true;
            }else {
            if(RAT[fetch_reg.src2] != -1) {
                int rob_idx = RAT[fetch_reg.src2];
                if (ROB[rob_idx].ready) {
                    entry.Valuek = ROB[rob_idx].value;
                    entry.Vk = true;
                } else {
                    entry.Tagk = rob_idx;
                    entry.Vk = false;
                }
            } else {
                entry.Valuek = ARF[fetch_reg.src2];
                entry.Vk = true;
            }
        }
            entry.dest = rob_tail;
            units[0].reservation_station.push_back(entry);
            RAT[fetch_reg.dest] = rob_tail;
            rob_tail = (rob_tail + 1) % ROB.size();
            rob_count++;
        }
        if(fetch_reg.op == OpCode::MUL) {
            if(units[1].isfull()) {
                is_stalled = true;
                return;
            }
            ROB[rob_tail].valid = true;
            ROB[rob_tail].ready = false;
            ROB[rob_tail].destReg = fetch_reg.dest;
            ROB[rob_tail].value = -1;
            RSEntry entry;
            entry.valid = true;
            entry.op = fetch_reg.op;
            if(RAT[fetch_reg.src1] != -1) {
                int rob_idx = RAT[fetch_reg.src1];
                if (ROB[rob_idx].ready) {
                    entry.Valuej = ROB[rob_idx].value;
                    entry.Vj = true;
                } else {
                    entry.Tagj = rob_idx;
                    entry.Vj = false;
                }
            } else {
                entry.Valuej = ARF[fetch_reg.src1];
                entry.Vj = true;
            }
            if(RAT[fetch_reg.src2] != -1) {
                int rob_idx = RAT[fetch_reg.src2];
                if (ROB[rob_idx].ready) {
                    entry.Valuek = ROB[rob_idx].value;
                    entry.Vk = true;
                } else {
                    entry.Tagk = rob_idx;
                    entry.Vk = false;
                }
            } else {
                entry.Valuek = ARF[fetch_reg.src2];
                entry.Vk = true;
            }
            entry.dest = rob_tail;
            units[1].reservation_station.push_back(entry);
            RAT[fetch_reg.dest] = rob_tail;
            rob_tail = (rob_tail + 1) % ROB.size();
            rob_count++;
        }
        if(fetch_reg.op == OpCode::DIV || fetch_reg.op == OpCode::REM) {
            if(units[2].isfull()) {
                is_stalled = true;
                return;
            }
            ROB[rob_tail].valid = true;
            ROB[rob_tail].ready = false;
            ROB[rob_tail].destReg = fetch_reg.dest;
            ROB[rob_tail].value = -1;
            RSEntry entry;
            entry.valid = true;
            entry.op = fetch_reg.op;
            if(RAT[fetch_reg.src1] != -1) {
                int rob_idx = RAT[fetch_reg.src1];
                if (ROB[rob_idx].ready) {
                    entry.Valuej = ROB[rob_idx].value;
                    entry.Vj = true;
                } else {
                    entry.Tagj = rob_idx;
                    entry.Vj = false;
                }
            } else {
                entry.Valuej = ARF[fetch_reg.src1];
                entry.Vj = true;
            }
            if(RAT[fetch_reg.src2] != -1) {
                int rob_idx = RAT[fetch_reg.src2];
                if (ROB[rob_idx].ready) {
                    entry.Valuek = ROB[rob_idx].value;
                    entry.Vk = true;
                } else {
                    entry.Tagk = rob_idx;
                    entry.Vk = false;
                }
            } else {
                entry.Valuek = ARF[fetch_reg.src2];
                entry.Vk = true;
            }
            entry.dest = rob_tail;
            units[2].reservation_station.push_back(entry);
            RAT[fetch_reg.dest] = rob_tail;
            rob_tail = (rob_tail + 1) % ROB.size();
            rob_count++;
        }
        if(fetch_reg.op == OpCode::BEQ || fetch_reg.op == OpCode::BNE || fetch_reg.op == OpCode::BLT || fetch_reg.op == OpCode::BLE) {
            if(units[3].isfull()) {
                is_stalled = true;
                return;
            }
            ROB[rob_tail].valid = true;
            ROB[rob_tail].ready = false;
            ROB[rob_tail].destReg = -1;
            ROB[rob_tail].value = -1;
            RSEntry entry;
            entry.valid = true;
            entry.op = fetch_reg.op;
            if(RAT[fetch_reg.src1] != -1) {
                int rob_idx = RAT[fetch_reg.src1];
                if (ROB[rob_idx].ready) {
                    entry.Valuej = ROB[rob_idx].value;
                    entry.Vj = true;
                } else {
                    entry.Tagj = rob_idx;
                    entry.Vj = false;
                }
            } else {
                entry.Valuej = ARF[fetch_reg.src1];
                entry.Vj = true;
            }
            if(RAT[fetch_reg.src2] != -1) {
                int rob_idx = RAT[fetch_reg.src2];
                if (ROB[rob_idx].ready) {
                    entry.Valuek = ROB[rob_idx].value;
                    entry.Vk = true;
                } else {
                    entry.Tagk = rob_idx;
                    entry.Vk = false;
                }
            } else {
                entry.Valuek = ARF[fetch_reg.src2];
                entry.Vk = true;
            }
            entry.dest = rob_tail;
            units[3].reservation_station.push_back(entry);
            rob_tail = (rob_tail + 1) % ROB.size();
            rob_count++;
        }
        if(fetch_reg.op == OpCode::AND || fetch_reg.op == OpCode::OR || fetch_reg.op == OpCode::XOR ||fetch_reg.op == OpCode::ANDI || fetch_reg.op == OpCode::ORI || fetch_reg.op == OpCode::XORI) {
            if(units[4].isfull()) {
                is_stalled = true;
                return;
            }
            ROB[rob_tail].valid = true;
            ROB[rob_tail].ready = false;
            ROB[rob_tail].destReg = fetch_reg.dest;
            ROB[rob_tail].value = -1;
        }
        if(fetch_reg.op == OpCode::SLT ||fetch_reg.op == OpCode::SLTI){
            if(units[0].isfull()) {
                is_stalled = true;
                return;
            }
            ROB[rob_tail].valid = true;
            ROB[rob_tail].ready = false;
            ROB[rob_tail].destReg = fetch_reg.dest;
            ROB[rob_tail].value = -1;
        }
    };

    void stageExecuteAndBroadcast();

    void stageCommit();

    bool step() {
        clock_cycle++;
        return true;
    }

    void dumpArchitecturalState() {
        std::cout << "\n=== ARCHITECTURAL STATE (CYCLE " << clock_cycle << ") ===\n";
        for (int i = 0; i < ARF.size(); i++) {
            std::cout << "x" << i << ": " << std::setw(4) << ARF[i] << " | ";
            if ((i+1) % 8 == 0) std::cout << std::endl;
        }
        if (exception) {
            std::cout << "EXCEPTION raised by instruction " << pc + 1 << std::endl;
        }
        std::cout << "Branch Predictor Stats: " << bp.correct_predictions << "/" << bp.total_branches << " correct.\n";
    }
};