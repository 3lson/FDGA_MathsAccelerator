TOP_MODULE=scalar_cpu
RTL_DIR=rtl
TB_DIR=tb

all: sim

sim: 
	verilator -Wall -Wno-UNUSEDSIGNAL \
		--top-module $(TOP_MODULE) \
		--cc $(RTL_DIR)/$(TOP_MODULE).sv \
        $(RTL_DIR)/fetch.sv \
        $(RTL_DIR)/decode.sv \
        $(RTL_DIR)/register_file.sv \
        $(RTL_DIR)/instruction_memory.sv \
        $(RTL_DIR)/program_counter.sv \
		--exe $(TB_DIR)/sim_main.cpp --build \
		-I$(RTL_DIR) -I$(TB_DIR)
	cp $(RTL_DIR)/instr_mem.mem obj_dir/
	./obj_dir/V$(TOP_MODULE)


clean:
	rm -rf obj_dir *.vcd
