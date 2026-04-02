#pragma once
#include <string>

enum class OpCode { ADD, SUB, ADDI, MUL, DIV, REM, LW, SW, BEQ, BNE, BLT, BLE, J, SLT, SLTI, AND, OR, XOR, ANDI, ORI, XORI };
enum class UnitType { ADDER, MULTIPLIER, DIVIDER, LOADSTORE, BRANCH, LOGIC };

struct Instruction {
    OpCode op;       
    int dest = -1;
    int src1 = -1;
    int src2 = -1;
    int imm = 0;
    int pc = -1;
};

struct ProcessorConfig {
    int num_regs = 32;
    int rob_size = 64;
    int mem_size = 1024;

    int logic_lat = 1;
    int add_lat = 2;
    int mul_lat = 4;
    int div_lat = 5;
    int mem_lat = 4;

    int logic_rs_size = 4;
    int adder_rs_size = 4;
    int mult_rs_size = 2;
    int div_rs_size = 2;
    int br_rs_size = 2;
    int lsq_rs_size = 32;
};

struct ROBEntry {
    // valid bit, ready bit, architectural register ID
    // other fields as required
    bool valid;
    bool ready;
    int destReg; // set -1 for branch instructions, -2 for store word
    int value;
    bool has_exception;
    int inst_number; // to set the pc if this rob entry caused exception and if this instruction was a branch instructions then need to check what was predicted to assert its correctness
};

struct RSEntry {
    bool valid;
    bool working = false;
    OpCode op;
    int Tagj = -1;
    int Tagk = -1;
    int Valuej = 0;
    int Valuek = 0;
    int dest = -1;
    int ROB_Entry; // the entry of this instruction in rob
    int current_latency = 0;
    bool Vj = false;
    bool Vk = false;
    long long sequence_number; // a sequence number to track the oldest instruciton, you can get this value from inst_counts in units
    int ind; // the slot index of RS Table in which this entry goes

    // value, tag, ready ... for both operands
    // other fields as required
};