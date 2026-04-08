#include "Processor.h"

void Processor::broadcastOnCDB() {
    for (int i = 0; i < units.size(); i++)
        units[i].capture(broadcast_tag, broadcast_value);
    lsq->capture(broadcast_tag, broadcast_value);
    ROB[broadcast_tag].value = broadcast_value;
    ROB[broadcast_tag].ready = true;
}

Instruction Processor::instParser(const std::string& line, long long line_number) {
    Instruction inst;
    string changed_line = line;
    replace(changed_line.begin(), changed_line.end(), ',', ' ');
    replace(changed_line.begin(), changed_line.end(), '(', ' ');
    replace(changed_line.begin(), changed_line.end(), ')', ' ');
    std::istringstream iss(changed_line);
    string word;
    iss >> word;
    inst.op = stringToOpCode(word);
    if (word == "add" || word == "sub" || word == "mul" || word == "div" || word == "rem" || word == "and" ||
        word == "or" || word == "xor" || word == "slt") {
        string rd, rs1, rs2;
        iss >> rd >> rs1 >> rs2;
        inst.dest = stoi(rd.substr(1));
        inst.src1 = stoi(rs1.substr(1));
        inst.src2 = stoi(rs2.substr(1));
    } else if (word == "addi" || word == "slti" || word == "andi" || word == "ori" || word == "xori") {
        string rd, rs1, imm;
        iss >> rd >> rs1 >> imm;
        inst.dest = stoi(rd.substr(1));
        inst.src1 = stoi(rs1.substr(1));
        inst.imm = stoi(imm);
    } else if (word == "lw") {
        string rd, offset, rs1;
        iss >> rd >> offset >> rs1;
        inst.dest = stoi(rd.substr(1));
        inst.src1 = stoi(rs1.substr(1));
        inst.imm = stoi(offset);
    } else if (word == "sw") {
        string rs2, offset, rs1;
        iss >> rs2 >> offset >> rs1;
        inst.src2 = stoi(rs2.substr(1));
        inst.src1 = stoi(rs1.substr(1));
        inst.imm = stoi(offset);
    } else if (word == "beq" || word == "bne" || word == "blt" || word == "ble") {
        string rs1, rs2, imm;
        iss >> rs1 >> rs2 >> imm;
        inst.src1 = stoi(rs1.substr(1));
        inst.src2 = stoi(rs2.substr(1));
        inst.imm = stoi(imm);
    } else if (word == "j") {
        string imm;
        iss >> imm;
        inst.imm = stoi(imm);
    }
    inst.pc = line_number;
    return inst;
}

OpCode Processor::stringToOpCode(const std::string& str) {
    if (str == "add")
        return OpCode::ADD;
    if (str == "sub")
        return OpCode::SUB;
    if (str == "addi")
        return OpCode::ADDI;
    if (str == "mul")
        return OpCode::MUL;
    if (str == "div")
        return OpCode::DIV;
    if (str == "rem")
        return OpCode::REM;
    if (str == "lw")
        return OpCode::LW;
    if (str == "sw")
        return OpCode::SW;
    if (str == "beq")
        return OpCode::BEQ;
    if (str == "bne")
        return OpCode::BNE;
    if (str == "blt")
        return OpCode::BLT;
    if (str == "ble")
        return OpCode::BLE;
    if (str == "j")
        return OpCode::J;
    if (str == "slt")
        return OpCode::SLT;
    if (str == "slti")
        return OpCode::SLTI;
    if (str == "and")
        return OpCode::AND;
    if (str == "or")
        return OpCode::OR;
    if (str == "xor")
        return OpCode::XOR;
    if (str == "andi")
        return OpCode::ANDI;
    if (str == "ori")
        return OpCode::ORI;
    if (str == "xori")
        return OpCode::XORI;
    throw std::invalid_argument("Invalid opcode: " + str);
}

void Processor::loadProgram(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    for (int i = 0; i < ARF.size(); i++)
        ARF[i] = 0;
    for (int i = 0; i < Memory.size(); i++)
        Memory[i] = 0;
    long long current_memory_index = 0;
    std::string line;
    long long line_number = 0;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;
        size_t start_index = (line[0] == '-') ? 1 : 0;
        if (line.length() <= start_index)
            continue;
        if (line.find_first_not_of("0123456789", start_index) == string::npos) {
            this->Memory[current_memory_index++] = stoi(line);
        } else {
            this->inst_memory.push_back(instParser(line, line_number));
            line_number++;
        }
    }
}
void Processor::InitializeROBEntry(bool valid, bool ready, int destReg, int value, int inst_number) {
    ROB[rob_tail].valid = valid;
    ROB[rob_tail].ready = ready;
    ROB[rob_tail].destReg = destReg;
    ROB[rob_tail].value = value;
    ROB[rob_tail].inst_number = inst_number;
    ROB[rob_tail].memory_addr = -1;
}
void Processor::setUpRSEntry(int src_reg, int& value, int& tag, bool& is_ready) {
    if (RAT[src_reg] != -1) {
        int rob_idx = RAT[src_reg];
        if (ROB[rob_idx].ready) {
            value = ROB[rob_idx].value;
            is_ready = true;
        } else {
            tag = rob_idx;
            is_ready = false;
        }
    } else {
        value = ARF[src_reg];
        is_ready = true;
    }
}
void Processor::stageFetch() {
    if (is_stalled)
        return;
    if (pc < inst_memory.size()) {
        fetch_reg = inst_memory[pc];
        pc = bp.predict(pc, fetch_reg.imm, fetch_reg.op);
        bp.predictionslist[fetch_reg.pc].push(pc);
    } else {
        fetch_reg.pc = -1;
    }
};
void Processor::stageDecode() {
    if (fetch_reg.pc == -1)
        return;
    if (rob_count == ROB.size()) {
        is_stalled = true;
        return;
    }
    if (fetch_reg.op == OpCode::ADD || fetch_reg.op == OpCode::SUB || fetch_reg.op == OpCode::ADDI) {
        if (units[0].isfull()) {
            is_stalled = true;
            return;
        }
        InitializeROBEntry(true, false, fetch_reg.dest, -1, fetch_reg.pc);
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        if (fetch_reg.op == OpCode::ADDI) {
            entry.Valuek = fetch_reg.imm;
            entry.Vk = true;
        } else {
            setUpRSEntry(fetch_reg.src2, entry.Valuek, entry.Tagk, entry.Vk);
        }
        entry.dest = rob_tail;
        entry.sequence_number = units[0].inst_counts;
        units[0].inst_counts++;
        int RS_index = units[0].available_ind.front();
        entry.ind = RS_index;
        units[0].reservation_station[RS_index] = entry;
        if (entry.Vj && entry.Vk)
            units[0].ready_inst.push(&units[0].reservation_station[RS_index]);
        units[0].available_ind.pop();
        if (fetch_reg.dest != 0)
            RAT[fetch_reg.dest] = rob_tail;
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::MUL) {
        if (units[1].isfull()) {
            is_stalled = true;
            return;
        }
        InitializeROBEntry(true, false, fetch_reg.dest, -1, fetch_reg.pc);
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        setUpRSEntry(fetch_reg.src2, entry.Valuek, entry.Tagk, entry.Vk);
        entry.dest = rob_tail;
        entry.sequence_number = units[1].inst_counts;
        units[1].inst_counts++;
        int RS_index = units[1].available_ind.front();
        entry.ind = RS_index;
        units[1].reservation_station[RS_index] = entry;
        if (entry.Vj && entry.Vk)
            units[1].ready_inst.push(&units[1].reservation_station[RS_index]);
        units[1].available_ind.pop();
        if (fetch_reg.dest != 0)
            RAT[fetch_reg.dest] = rob_tail;
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::DIV || fetch_reg.op == OpCode::REM) {
        if (units[2].isfull()) {
            is_stalled = true;
            return;
        }
        InitializeROBEntry(true, false, fetch_reg.dest, -1, fetch_reg.pc);
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        setUpRSEntry(fetch_reg.src2, entry.Valuek, entry.Tagk, entry.Vk);
        entry.dest = rob_tail;
        entry.sequence_number = units[2].inst_counts;
        units[2].inst_counts++;
        int RS_index = units[2].available_ind.front();
        entry.ind = RS_index;
        units[2].reservation_station[RS_index] = entry;
        if (entry.Vj && entry.Vk)
            units[2].ready_inst.push(&units[2].reservation_station[RS_index]);
        units[2].available_ind.pop();
        if (fetch_reg.dest != 0)
            RAT[fetch_reg.dest] = rob_tail;
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::BEQ || fetch_reg.op == OpCode::BNE || fetch_reg.op == OpCode::BLT ||
        fetch_reg.op == OpCode::BLE) {
        if (units[3].isfull()) {
            is_stalled = true;
            return;
        }
        InitializeROBEntry(true, false, -1, -1, fetch_reg.pc);
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        setUpRSEntry(fetch_reg.src2, entry.Valuek, entry.Tagk, entry.Vk);
        entry.dest = rob_tail;
        entry.sequence_number = units[3].inst_counts;
        units[3].inst_counts++;
        int RS_index = units[3].available_ind.front();
        entry.ind = RS_index;
        units[3].reservation_station[RS_index] = entry;
        if (entry.Vj && entry.Vk)
            units[3].ready_inst.push(&units[3].reservation_station[RS_index]);
        units[3].available_ind.pop();
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::AND || fetch_reg.op == OpCode::OR || fetch_reg.op == OpCode::XOR ||
        fetch_reg.op == OpCode::ANDI || fetch_reg.op == OpCode::ORI || fetch_reg.op == OpCode::XORI) {
        if (units[4].isfull()) {
            is_stalled = true;
            return;
        }
        InitializeROBEntry(true, false, fetch_reg.dest, -1, fetch_reg.pc);
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        if (fetch_reg.op == OpCode::ANDI || fetch_reg.op == OpCode::ORI || fetch_reg.op == OpCode::XORI) {
            entry.Valuek = fetch_reg.imm;
            entry.Vk = true;
        } else {
            setUpRSEntry(fetch_reg.src2, entry.Valuek, entry.Tagk, entry.Vk);
        }
        entry.dest = rob_tail;
        entry.sequence_number = units[4].inst_counts;
        units[4].inst_counts++;
        int RS_index = units[4].available_ind.front();
        entry.ind = RS_index;
        units[4].reservation_station[RS_index] = entry;
        units[4].available_ind.pop();
        if (entry.Vj && entry.Vk)
            units[4].ready_inst.push(&units[4].reservation_station[RS_index]);
        if (fetch_reg.dest != 0)
            RAT[fetch_reg.dest] = rob_tail;
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::SLT || fetch_reg.op == OpCode::SLTI) {
        if (units[0].isfull()) {
            is_stalled = true;
            return;
        }
        InitializeROBEntry(true, false, fetch_reg.dest, -1, fetch_reg.pc);
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        if (fetch_reg.op == OpCode::SLTI) {
            entry.Valuek = fetch_reg.imm;
            entry.Vk = true;
        } else {
            setUpRSEntry(fetch_reg.src2, entry.Valuek, entry.Tagk, entry.Vk);
        }
        entry.dest = rob_tail;
        entry.sequence_number = units[0].inst_counts;
        units[0].inst_counts++;
        int RS_index = units[0].available_ind.front();
        entry.ind = RS_index;
        units[0].reservation_station[RS_index] = entry;
        if (entry.Vj && entry.Vk)
            units[0].ready_inst.push(&units[0].reservation_station[RS_index]);
        units[0].available_ind.pop();
        if (fetch_reg.dest != 0)
            RAT[fetch_reg.dest] = rob_tail;
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::LW) {
        if (lsq->isfull()) {
            is_stalled = true;
            return;
        }
        ROB[rob_tail].valid = true;
        ROB[rob_tail].ready = false;
        ROB[rob_tail].destReg = fetch_reg.dest;
        ROB[rob_tail].value = -1;
        ROB[rob_tail].inst_number = fetch_reg.pc;
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        entry.Vk = true;
        entry.imm = fetch_reg.imm;
        entry.dest = rob_tail;
        entry.sequence_number = lsq->inst_counts;
        lsq->inst_counts++;
        entry.ind = lsq->available_ind.front();
        lsq->reservation_station[lsq->available_ind.front()] = entry;
        if (entry.Vj && entry.Vk)
                lsq->ready_inst.push(&lsq->reservation_station[lsq->available_ind.front()]);
        lsq->available_ind.pop();
        // lsq->lsq_queue[lsq->end_index] = entry;
        // lsq->end_index = (lsq->end_index + 1) % lsq->rs_size;
        if (fetch_reg.dest != 0)
            RAT[fetch_reg.dest] = rob_tail;
        // RAT[fetch_reg.dest] = rob_tail;
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::SW) {
        if (lsq->isfull()) {
            is_stalled = true;
            return;
        }
        InitializeROBEntry(true, false, -2, -1, fetch_reg.pc);
        RSEntry entry;
        entry.valid = true;
        entry.op = fetch_reg.op;
        entry.imm = fetch_reg.imm;
        entry.sequence_number = lsq->inst_counts;
        lsq->inst_counts++;
        setUpRSEntry(fetch_reg.src1, entry.Valuej, entry.Tagj, entry.Vj);
        setUpRSEntry(fetch_reg.src2, entry.Valuek, entry.Tagk, entry.Vk);
        entry.dest = rob_tail;
        entry.ind = lsq->available_ind.front();
        lsq->reservation_station[lsq->available_ind.front()] = entry;
        lsq->available_ind.pop();
        // lsq->lsq_queue[lsq->end_index] = entry;
        // lsq->end_index = (lsq->end_index + 1) % lsq->rs_size;
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
    if (fetch_reg.op == OpCode::J) {
        if (units[3].isfull()) {
            is_stalled = true;
            return;
        }
        ROB[rob_tail].valid = true;
        ROB[rob_tail].ready = true;
        ROB[rob_tail].destReg = -3;
        ROB[rob_tail].value = -1;
        ROB[rob_tail].inst_number = fetch_reg.pc;
        // RSEntry entry;
        // entry.valid = true;
        // entry.op = fetch_reg.op;
        // entry.Valuej = fetch_reg.imm;
        // entry.Vj = true;
        // entry.dest = rob_tail;
        // int RS_index = units[3].available_ind.front();
        // units[3].reservation_station[RS_index] = entry;
        // units[3].available_ind.pop();
        rob_tail = (rob_tail + 1) % ROB.size();
        rob_count++;
        is_stalled = false;
    }
}

void Processor::flush() {
    while (ROB[rob_head].valid) {
        ROB[rob_head].valid = false;
        rob_head++;
        rob_head %= ROB.size();
    }
    rob_head = 0;
    rob_tail = 0;
    rob_count = 0;
    for (int i = 0; i < units.size(); i++) {
        for (auto& entry : units[i].reservation_station) {
            entry.valid = false;
            entry.Vj = false;
            entry.Vk = false;
            entry.current_latency = 0;
            entry.working = false;
            entry.inQueue = false;
        }
        units[i].pipeline.clear();
        while (!units[i].ready_inst.empty())
            units[i].ready_inst.pop();
        while (!units[i].available_ind.empty())
            units[i].available_ind.pop();
        for (int j = 0; j < units[i].reservation_station.size(); j++)
            units[i].available_ind.push(j);
        units[i].has_exception = false;
        units[i].has_result = false;
        units[i].result_value = 0;
        units[i].inst_counts = 0;
    }
    for (auto& entry : lsq->reservation_station) {
        entry.valid = false;
        entry.Vj = false;
        entry.Vk = false;
        entry.current_latency = 0;
        entry.working = false;
        entry.inQueue = false;
    }
    lsq->pipeline.clear();
    while (!lsq->ready_inst.empty())
        lsq->ready_inst.pop();
    while (!lsq->available_ind.empty())
        lsq->available_ind.pop();
    for (int j = 0; j < lsq->reservation_station.size(); j++)
        lsq->available_ind.push(j);
    lsq->has_exception = false;
    lsq->has_result = false;
    lsq->result_value2 = 0;
    lsq->result_value = 0;
    lsq->lsq_inst = 0;
    lsq->inst_counts = 0;
    lsq->uncommitedSw.clear();
    lsq->isSW = false;
    for (int i = 0; i < RAT.size(); i++)
        RAT[i] = -1;
    for (int i = 0; i < bp.predictionslist.size(); i++)
        while (!bp.predictionslist[i].empty())
            bp.predictionslist[i].pop();
}

void Processor::stageExecuteAndBroadcast() {
    std::vector<std::pair<int, int>> broadcasts;
    for (int i = 0; i < units.size(); i++) {
        units[i].executeCycle();
        if (units[i].has_exception)
            ROB[units[i].result_tag].has_exception = true;
        if (units[i].has_result) {
            broadcasts.push_back({units[i].result_tag, units[i].result_value});
            broadcastOnCDB();
        }
    }
    lsq->executeCycle(Memory);
    if (lsq->has_exception)
        ROB[lsq->result_tag].has_exception = true;
    if (lsq->has_result) {
        if (lsq->isSW)
            ROB[lsq->result_tag].memory_addr = lsq->result_value2;
        broadcasts.push_back({lsq->result_tag, lsq->result_value});
        broadcastOnCDB();
    }
    for (auto& p : broadcasts) {
        broadcast_tag = p.first;
        broadcast_value = p.second;
        broadcastOnCDB();
    }
}

void Processor::stageCommit() {
    if (!ROB[rob_head].valid || !ROB[rob_head].ready)
        return;
    if (ROB[rob_head].has_exception) {
        exception = true;
        pc = ROB[rob_head].inst_number;
        flush();
        return;
    }
    if (ROB[rob_head].destReg == -1) {  // this means it was a branch instruction's entry
        int state = bp.predictionslist[ROB[rob_head].inst_number].front();
        bp.predictionslist[ROB[rob_head].inst_number].pop();
        if (ROB[rob_head].value == 1) {
            if (state == 1) {
                bp.instruction_state[ROB[rob_head].inst_number] = 0;
                bp.correct_predictions++;
            } else if (state == 2) {
                bp.instruction_state[ROB[rob_head].inst_number] = 1;
                flush();
                pc = inst_memory[ROB[rob_head].inst_number].imm;
            } else if (state == 3) {
                bp.instruction_state[ROB[rob_head].inst_number] = 2;
                flush();
                pc = inst_memory[ROB[rob_head].inst_number].imm;
            } else {
                bp.correct_predictions++;
            }
        } else {
            if (state == 0) {
                bp.instruction_state[ROB[rob_head].inst_number] = 1;
                flush();
                pc = ROB[rob_head].inst_number + 1;
            } else if (state == 1) {
                flush();
                pc = ROB[rob_head].inst_number + 1;
                bp.instruction_state[ROB[rob_head].inst_number] = 2;
            } else if (state == 2) {
                bp.instruction_state[ROB[rob_head].inst_number] = 3;
                bp.correct_predictions++;
            } else if (state == 3) {
                bp.correct_predictions++;
            }
        }
    } else if (ROB[rob_head].destReg == -2) {
        if (lsq->uncommitedSw[ROB[rob_head].memory_addr].second == rob_head)
            lsq->uncommitedSw.erase(ROB[rob_head].memory_addr);
        Memory[ROB[rob_head].memory_addr] = ROB[rob_head].value;
    } else if (ROB[rob_head].destReg > 0) {  // 0th register is always 0
        ARF[ROB[rob_head].destReg] = ROB[rob_head].value;
        if (RAT[ROB[rob_head].destReg] == rob_head)
            RAT[ROB[rob_head].destReg] = -1;
    }
    ROB[rob_head].valid = false;
    rob_head++;
    rob_head %= ROB.size();
    rob_count--;
}