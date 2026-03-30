#include "Processor.h"

void Processor::broadcastOnCDB() {
    
}

void Processor::flush() {

}

void Processor::stageFetch() {
    
}

void Processor::stageDecode() {

}

void Processor::stageExecuteAndBroadcast() {
    for (int i = 0; i < units.size(); i++) {
        units[i].executeCycle();
    }
}

void Processor::stageCommit() {
    
}