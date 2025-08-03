# This file contains the minimal constraints needed to run synthesis and
# timing analysis on the GPU core using the synth_wrapper.
# It is derived from the official PYNQ-Z1 master XDC file.

# 1. System Clock (125 MHz)
set_property -dict { PACKAGE_PIN L16   IOSTANDARD LVCMOS33 } [get_ports clk]
create_clock -period 8.000 -name sys_clk [get_ports clk]

# 2. System Reset (BTN0)
set_property -dict { PACKAGE_PIN R18   IOSTANDARD LVCMOS33 } [get_ports reset]

# 3. GPU Done Output (LD0)
# This gives the synthesizer a reason to keep the logic driving the 'done' signal.
set_property -dict { PACKAGE_PIN M14   IOSTANDARD LVCMOS33 } [get_ports gpu_done_led]