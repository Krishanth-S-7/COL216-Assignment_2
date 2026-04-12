#include "LoadStoreQueue.h"

void LoadStoreQueue::capture(int tag, int val) {
    for (int i = 0; i < reservation_station.size(); i++) {
        if (!reservation_station[i].valid)
            continue;
        if (!reservation_station[i].Vj && reservation_station[i].Tagj == tag) {
            reservation_station[i].Vj = true;
            reservation_station[i].Valuej = val;
        }
        if (!reservation_station[i].Vk && reservation_station[i].Tagk == tag) {
            reservation_station[i].Vk = true;
            reservation_station[i].Valuek = val;
        }
        if (!reservation_station[i].working && !reservation_station[i].inQueue && reservation_station[i].Vj &&
            reservation_station[i].Vk) {
            ready_inst.push(&reservation_station[i]);
            reservation_station[i].inQueue = true;
        }
    }
}

void LoadStoreQueue::executeCycle(std::vector<int>& Memory) {
    has_result = false;
    has_exception = false;
    result_value = 0;
    isSW = false;
    if (indToFree != -1) {
        reservation_station[indToFree].valid = false;
        reservation_station[indToFree].Vj = false;
        reservation_station[indToFree].Vk = false;
        reservation_station[indToFree].inQueue = false;
        reservation_station[indToFree].working = false;
        reservation_station[indToFree].current_latency = 0;
        available_ind.push(indToFree);
    }
    indToFree = -1;
    pushIntoPipeline();
    for (int i = 0; i < pipeline.size(); i++)
        pipeline[i]->current_latency = std::min(pipeline[i]->current_latency + 1, latency - i);
    if (!pipeline.empty() && pipeline.front()->current_latency == latency) {
        runInst(Memory);
    }
}

void LoadStoreQueue::pushIntoPipeline() {
    if (pipeline.size() == latency || ready_inst.empty() || ready_inst.top()->sequence_number != lsq_inst)
        return;
    pipeline.push_back(ready_inst.top());
    ready_inst.top()->working = true;
    ready_inst.top()->current_latency = 0;
    ready_inst.pop();
    lsq_inst++;
}

void LoadStoreQueue::removeEntry() {
    indToFree = pipeline.front()->ind;
    // if (pipeline.empty()) return;
    // reservation_station[pipeline.front()->ind].valid = false;
    // reservation_station[pipeline.front()->ind].Vj = false;
    // reservation_station[pipeline.front()->ind].Vk = false;
    // reservation_station[pipeline.front()->ind].inQueue = false;
    // reservation_station[pipeline.front()->ind].working = false;
    // reservation_station[pipeline.front()->ind].current_latency = 0;
    // available_ind.push(pipeline.front()->ind);
}

bool LoadStoreQueue::isfull() { return available_ind.empty(); }

void LoadStoreQueue::loadstore(std::vector<int>& Memory) {
    RSEntry* inst = pipeline.front();
    if (inst->op == OpCode::LW) {
        int addr = inst->Valuej + inst->imm;
        if (addr < 0 || addr >= Memory.size()) {
            has_exception = true;
            has_result = true;
            result_tag = pipeline.front()->dest;
            removeEntry();
            pipeline.pop_front();
        } else {
            if (uncommitedSw.find(addr) == uncommitedSw.end()) {
                result_value = Memory[addr];
                has_result = true;
                result_tag = pipeline.front()->dest;
                removeEntry();
                pipeline.pop_front();
            } else {
                result_value = uncommitedSw[addr].first;
                has_result = true;
                result_tag = inst->dest;
                removeEntry();
                pipeline.pop_front();
            }
        }
    } else if (inst->op == OpCode::SW) {
        isSW = true;
        result_value2 = inst->Valuej + inst->imm;
        result_value = inst->Valuek;
        if (result_value2 < 0 || result_value2 >= Memory.size())
            has_exception = true;
        else
            uncommitedSw[result_value2] = {result_value, inst->dest};
        has_result = true;
        result_tag = pipeline.front()->dest;
        removeEntry();
        pipeline.pop_front();
    }
}
void LoadStoreQueue::runInst(std::vector<int>& Memory) { loadstore(Memory); }