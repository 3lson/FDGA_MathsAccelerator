## Lanes

For each lane, we should internally pipeline with a 4-stage pipeline:

ID -> EX -> MEM -> WB

MEM stage is not directly necessary but we can implement it for now and remove it later. Only used for load/stores.

The lane will now include a floating point and regular alu. The processor will then contain the registers and work with scheduling to interface with each register.




Okay, let's outline the steps for Option 1: Building the SIMT Unit as a Coprocessor, controlled by your existing RISC-V CPU.

**Core Idea:** Your RISC-V CPU (the "Host") will set up tasks for the SIMT Coprocessor (the "Device"). The SIMT Coprocessor then executes these tasks in parallel.

**Overall System Diagram (Option 1):**

```
+--------------------------+      Control/Status      +----------------------------+
|   RISC-V CPU (Host)      |<----------------------->| SIMT Coprocessor (Device)  |
| (Your existing core from |      (MMIO Registers)    |                            |
|  GitHub, running main    |                          | +------------------------+ |
|  control program)        |                          | | simt_warp_scheduler  | |
+--------------------------+                          | +------------------------+ |
          ^                                           | +------------------------+ |
          |                                           | | simt_datapath_core     | |
          |                                           | |  (N x lane_pipelined)  | |
          | (Access to shared memory)                 | |  (simt_reg_file_sys) | |
          V                                           | +------------------------+ |
+---------------------------------------------------+ | +------------------------+ |
|                 Main Memory System                | | | simt_memory_unit     | |
|                (e.g., DDR via AXI)                |<->| (Accesses Main Memory) | |
+---------------------------------------------------+ +------------------------+ |
                                                      +----------------------------+
```

---

**Phase 1: Defining the Interface and Basic SIMT Structure**

**Step 1: Define Host-Device Interface (RISC-V CPU <-> SIMT Coprocessor)**

*   **Memory-Mapped I/O (MMIO) Registers:** This is the most common way. Your RISC-V CPU will write to and read from specific memory addresses that are routed to control/status registers within the SIMT coprocessor.
    *   **Registers to Design in SIMT Coprocessor (accessible by Host):**
        *   `SIMT_CTRL_REG` (Write):
            *   `start_kernel` (bit): Host writes 1 to start. Coprocessor clears when done or on ack.
            *   `reset_simt` (bit): To reset the SIMT unit.
        *   `SIMT_STATUS_REG` (Read):
            *   `busy` (bit): 1 if SIMT unit is executing a kernel.
            *   `kernel_done` (bit): Pulsed high or latched when kernel finishes.
            *   `error_code` (bits): For reporting SIMT errors.
        *   `SIMT_KERNEL_PC_START_REG` (Write): Host writes the starting address (in Main Memory) of the SIMT kernel code.
        *   `SIMT_NUM_BLOCKS_REG` (Write): Number of thread blocks to launch.
        *   `SIMT_THREADS_PER_BLOCK_REG` (Write): Number of threads in each block.
        *   `SIMT_BLOCK_DIM_X_REG`, `SIMT_BLOCK_DIM_Y_REG`, `SIMT_BLOCK_DIM_Z_REG` (Write): Dimensions of a block (if you support multi-dimensional blocks). For K-means, 1D is fine.
        *   `SIMT_ARG_PTR_REG_0`, `SIMT_ARG_PTR_REG_1`, ... (Write): Pointers to kernel arguments (e.g., pointers to `points_x`, `centroids_x` arrays in Main Memory).
    *   **Action:** Design a small module within the SIMT coprocessor to implement these registers and the logic to connect them to an AXI-Lite slave interface (or a simpler bus if your CPU's memory interface allows direct connection to peripherals). Your RISC-V CPU's `dmem_interface` would need to be able to route accesses to these MMIO addresses.

**Step 2: Define a Basic SIMT Instruction Set Architecture (ISA)**

*   Keep it simple initially.
*   **Data Types:** 32-bit integers, 32-bit floats.
*   **Registers:** Per thread: 28 GPRs (x0-x27), 4 Special Registers (x28-x31 for tIdx, bIdx, bDim, lId).
*   **Instruction Format:** Similar to RISC-V (e.g., R-type, I-type).
*   **Basic Instructions (Vectorized for SIMT lanes):**
    *   `VLOAD Vd, [Vs1 + imm]`: Lane loads from memory address (Vs1 + imm) into its Vd. Address calculation might be `base_for_warp + tIdx_in_warp * element_size`.
    *   `VSTORE Vs2, [Vs1 + imm]`: Lane stores Vs2 to memory.
    *   `VADD Vd, Vs1, Vs2`
    *   `VSUB Vd, Vs1, Vs2`
    *   `VFADD Vd, Vs1, Vs2` (Floating Point)
    *   `VFSUB Vd, Vs1, Vs2`
    *   `VFMUL Vd, Vs1, Vs2`
    *   `VFDIV Vd, Vs1, Vs2`
    *   `VFABS Vd, Vs1`
    *   `VIMM Vd, imm`: Load immediate into Vd for all active lanes.
    *   `VCMP_LT Vd, Vs1, Vs2`: Compare, set Vd to 1 if true, 0 if false (per lane).
    *   `BRANCH_IF_ANY target_offset`: If any active lane's condition (e.g., from a previous VCMP in a condition register) is true, branch. (More complex: `BRANCH_IF_ALL`).
    *   `BRANCH_IF_ZERO Vs1, target_offset`: If Vs1 is zero for an active lane, it wants to branch.
    *   `BARRIER`: Synchronize threads within a warp (or block, if more complex).
    *   `HALT_WARP`: Stops execution for the current warp.
*   **Action:** Document these instructions, their opcodes, and formats.

**Step 3: Design Core SIMT Lane Components (If not already finalized)**

*   **`lane_pipelined.sv`:**
    *   Ensure it has ID, EX, MEM (passthrough for now), WB stages.
    *   Takes inputs like `instr_FUNCT4`, `instr_IMM`, `instr_AD1_rs1`, etc.
    *   Takes `instr_WE3` (from `active_mask`).
    *   Interfaces with an RF for operand reads and result writes.
    *   Instantiates your ALU and FPU.
*   **`per_lane_thread_context_rf.sv`:**
    *   Stores GPRs (x0-x27).
    *   Provides access to special registers (x28-x31) whose values are fed from the Warp Scheduler.
*   **`simt_register_file_system.sv`:**
    *   Instantiates `NUM_SIMT_LANES` of `per_lane_thread_context_rf`.
    *   Handles routing of common read addresses and per-lane write data.
*   **Action:** Solidify these three modules. They are the building blocks for the datapath.

---

**Phase 2: Building the SIMT Datapath and Basic Control**

**Step 4: Design `simt_datapath_core.sv`**

*   **Purpose:** Instantiate and connect the SIMT lanes and their register file system.
*   **Inputs:** From Warp Scheduler (decoded instruction parts, active mask, special reg values).
*   **Outputs:** Aggregate stall signal to Warp Scheduler.
*   **Internal:**
    *   Instantiate `simt_register_file_system`.
    *   Generate `NUM_SIMT_LANES` instances of `lane_pipelined`.
    *   **Connections:**
        *   Scheduler's common decoded fields (`common_instr_ad1_rs1`, `common_instr_funct4`, etc.) go to all `lane_pipelined` inputs.
        *   Scheduler's `active_mask_per_lane[i]` goes to `lane_pipelined[i].instr_WE3`.
        *   Scheduler's special register values (`assigned_tIdx_per_lane[i]`, etc.) go to the `simt_register_file_system`.
        *   Wire up the RF read/write paths between each `lane_pipelined[i]` and the `simt_register_file_system` (for lane `i`'s context).
*   **Action:** Create this structural module. Test with simple instruction flow (e.g., a VADD).

**Step 5: Design `simt_warp_scheduler.sv` (Initial, Single Warp, No Divergence)**

*   **Purpose:** Fetch SIMT instructions, decode them, control the `simt_datapath_core`, and manage a single warp.
*   **Parameters:** `NUM_SIMT_LANES` (e.g., 4 or 8).
*   **State:**
    *   `warp_pc`: Program counter for the SIMT kernel code.
    *   `warp_active`: Is this warp currently running?
    *   `current_block_idx`, `current_threads_in_this_block`.
*   **Inputs:**
    *   `clk`, `rst_n`.
    *   From Host Interface: `start_kernel_signal`, `kernel_pc_start_val`, `num_blocks_val`, `threads_per_block_val`, argument pointers.
    *   From SIMT Instruction Memory: `simt_instr_data_i`.
    *   From `simt_datapath_core`: `aggregate_datapath_stall_o`.
*   **Outputs:**
    *   To Host Interface: `simt_busy_o`, `kernel_done_o`.
    *   To SIMT Instruction Memory: `simt_instr_addr_o` (this is `warp_pc`).
    *   To `simt_datapath_core`: Decoded instruction fields, `active_mask_per_lane`, `assigned_tIdx_per_lane`, etc.
*   **Logic (Simplified Initial Version):**
    1.  **Initialization:** Wait for `start_kernel_signal`. Load `kernel_pc_start_val` into `warp_pc`. Set `simt_busy_o`. Calculate initial `assigned_tIdx_per_lane` (e.g., `0` to `NUM_SIMT_LANES-1`). Set `active_mask_per_lane` to all 1s (assuming `threads_per_block >= NUM_SIMT_LANES`). Set `bIdx`, `bDim`.
    2.  **Instruction Fetch:** Send `warp_pc` to SIMT Instruction Memory.
    3.  **Decode:** Decode `simt_instr_data_i` into common fields and control signals.
    4.  **Issue:** If not stalled, send decoded parts to `simt_datapath_core`. Increment `warp_pc` (unless branch).
    5.  **HALT_WARP:** If decoded, set `kernel_done_o`, clear `simt_busy_o`, stop fetching.
    6.  **No Divergence Handling Yet:** Assume all threads in the warp follow the same path.
*   **Action:** Implement this basic scheduler. It will need to read SIMT instructions from somewhere (initially, maybe a simple ROM or a BRAM preloaded via the host).

**Step 6: SIMT Instruction Memory**

*   **Purpose:** Store the SIMT kernel code.
*   **Implementation:** For now, a simple BRAM.
*   **Interface:** Address input (from `simt_warp_scheduler.simt_instr_addr_o`), data output (`simt_instr_data_i`).
*   **Loading:** The Host CPU needs a way to write the SIMT kernel code into this BRAM before starting the kernel. This could be another set of MMIO registers for address and data, and a write enable.
*   **Action:** Implement this BRAM and the Host interface to load it.

---

**Phase 3: Memory Access and Integration**

**Step 7: Design `simt_memory_unit.sv` (Basic)**

*   **Purpose:** Allow SIMT lanes to access the Main Memory System.
*   **Inputs:**
    *   From `simt_warp_scheduler` (or directly from lanes if you choose that, but scheduler is easier to start): `mem_req`, `is_load`, `is_store`, `simt_mem_addr_per_active_lane [NUM_SIMT_LANES-1:0][31:0]`, `data_to_store_per_active_lane [NUM_SIMT_LANES-1:0][31:0]`, `active_mask_for_mem_op`.
*   **Outputs:**
    *   To `simt_warp_scheduler`: `mem_stall_o`, `data_loaded_per_active_lane [NUM_SIMT_LANES-1:0][31:0]`, `mem_op_done`.
    *   To Main Memory System: AXI (or other bus) master signals.
*   **Logic (Simplified Initial Version - serialized access):**
    1.  When `mem_req` is asserted:
    2.  Iterate through active lanes (based on `active_mask_for_mem_op`).
    3.  For each active lane `i`:
        *   Issue a memory request to Main Memory using `simt_mem_addr_per_active_lane[i]`.
        *   If store, send `data_to_store_per_active_lane[i]`.
        *   Wait for acknowledgment from Main Memory.
        *   If load, buffer `data_loaded_per_active_lane[i]`.
    4.  Signal `mem_op_done` when all active lane requests are complete.
    5.  Assert `mem_stall_o` to the scheduler while busy.
*   **Action:** Implement this. The interface to Main Memory (e.g., AXI Master) will be a significant part. For K-Means, lanes will need to load `points_x[i]`, `points_y[i]`, and `centroids_x[j]`, `centroids_y[j]`. And store back to `clusters_x`, `clusters_y`.

**Step 8: Integration and Host Control Program**

*   **Connect all SIMT Coprocessor Modules:** `simt_warp_scheduler`, `simt_datapath_core`, `simt_memory_unit`, SIMT Instruction Memory, Host MMIO Interface.
*   **Write Host Program (RISC-V Assembly/C):**
    1.  Load SIMT kernel binary into SIMT Instruction Memory (via MMIO).
    2.  Load input data (e.g., `points_x`, `points_y` for K-Means) into Main Memory at known locations.
    3.  Write kernel argument pointers (pointing to data in Main Memory) to `SIMT_ARG_PTR_REG_...`.
    4.  Write `SIMT_KERNEL_PC_START_REG`, `SIMT_NUM_BLOCKS_REG`, `SIMT_THREADS_PER_BLOCK_REG`.
    5.  Write 1 to `SIMT_CTRL_REG.start_kernel`.
    6.  Poll `SIMT_STATUS_REG.busy` until it's 0, or wait for `SIMT_STATUS_REG.kernel_done`.
    7.  Read results from Main Memory (where the SIMT kernel stored them).
*   **Write SIMT Kernel (Assembly for your new SIMT ISA):**
    *   Implement the K-Means algorithm using your defined SIMT instructions.
    *   Access `tIdx`, `bIdx`, etc., from special registers to compute memory addresses and manage work distribution.
    *   Use `VLOAD`/`VSTORE` to interact with data passed by pointers.
*   **Action:** Perform the top-level integration. Write a very simple SIMT kernel (e.g., vector add) and a host program to test the entire flow.

---

**Phase 4: Refinement and Advanced Features (Iterative)**

*   **Divergence Handling in Warp Scheduler:** Implement a predication stack or other mechanisms to handle `if/else` constructs where threads in a warp diverge.
*   **Multi-Warp Scheduling:** Allow the scheduler to manage multiple warps to hide latency (e.g., switch to another ready warp if the current one stalls on memory).
*   **Improved Memory Unit:** Coalescing memory accesses, handling more parallelism.
*   **Barriers:** Implement `BARRIER` instruction for synchronization.
*   **Atomic Operations:** If needed for algorithms like K-Means' `cluster_sizes++`.
*   **Compiler/Assembler:** Eventually, you'll want tools for your SIMT ISA.

**Starting Small:**

*   `NUM_SIMT_LANES = 2` or `4`.
*   Very simple SIMT ISA.
*   Single warp scheduler, no divergence.
*   Serialized memory access in `simt_memory_unit`.

This is a significant undertaking. Break it down, test each module thoroughly, and build up complexity incrementally. Good luck!