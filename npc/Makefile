build:
	verilator -Wall --cc --trace --exe --build csrc/tb_top.cpp vsrc/top.v

sim: build
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	./obj_dir/Vtop

clean:
	rm -rf obj_dir/

include ../Makefile
