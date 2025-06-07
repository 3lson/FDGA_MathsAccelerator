## Refactor edits

### As of 6 Jun 2025

**Dispatcher**
- doit.sh added the num_cores as it is specified by us

**ALU**
- add clk back as the clk is needed as implicit latch for the result 
- normally in 5-stage pipeline the pipeline registers are explicit for latching in this case don't have these registers so clk it
- latch means: A clocked flip-flop or set of flip-flops that captures the output of a stage (e.g., ALU) at the end of a cycle, passing it to the next stage.

- issue: putting the two together for I type and R type the cases overlap

- pc calculation done in ALU
