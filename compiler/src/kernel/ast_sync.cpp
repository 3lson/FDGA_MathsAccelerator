#include "../../include/kernel/ast_sync.hpp"
#include <iostream>

namespace ast {

void SyncStatement::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    (void) context;
    (void) dest_reg;

    // A 'sync' statement in the C code translates to a SINGLE 'sync' instruction
    // in the assembly. The hardware's control unit is responsible for implementing
    // the barrier logic (i.e., stalling the warp until all threads arrive).
    // The compiler's job is simply to emit the opcode.
    
    // We create the label as the instruction might require it, even if the hardware
    // handles the rest.
    std::string endsync_label = context.create_label("endsync");

    // The 'sync' instruction tells the hardware to pause this warp until all
    // active threads in the warp have also reached this instruction.
    stream << "sync " << endsync_label << std::endl;
    
    // The label marks the instruction immediately following the sync barrier.
    // When the hardware releases the warp, all threads will resume execution from here.
    stream << endsync_label << ": "  << std::endl;
}

void SyncStatement::Print(std::ostream& stream) const {
    stream << "sync;" <<  std::endl;
}

}
