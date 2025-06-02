## Lanes

For each lane, we should internally pipeline with a 4-stage pipeline:

ID -> EX -> MEM -> WB

MEM stage is not directly necessary but we can implement it for now and remove it later. Only used for load/stores.

The lane will now include a floating point and regular alu. The processor will then contain the registers and work with scheduling to interface with each register.