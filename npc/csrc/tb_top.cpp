#include <verilated.h>
#include <verilated_vcd_c.h>

#include <stdio.h>
#include <assert.h>

#include "Vtop.h"

#define MAX_SIM_TIME 20

int main(int argc, char** argv, char** env) {
    // Using unique_ptr is similar to
    // "VerilatedContext* contextp = new VerilatedContext" then deleting at end.
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};

    // Construct the Verilated model, from Vtop.h generated from Verilating "top.v".
    // Using unique_ptr is similar to "Vtop* top = new Vtop" then deleting at end.
    // "TOP" will be the hierarchical name of the module.
    const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};


    // Verilated::traceEverOn(true);
    // Verilator must compute traced signals
    contextp->traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;  // trace file pointer
    top->trace(tfp, 99);  // Trace 99 levels of hierarchy
    tfp->open("waveform.vcd");

    while (contextp->time() < MAX_SIM_TIME && !contextp->gotFinish()) {
        contextp->timeInc(1);

        int a = rand() & 1;
        int b = rand() & 1;
        top->a = a;
        top->b = b;

        // Evaluate model
        // (If you have multiple models being simulated in the same
        // timestep then instead of eval(), call eval_step() on each, then
        // eval_end_step() on each. See the manual.)
        top->eval();

        tfp->dump(contextp->time());

        printf("a = %d, b = %d, f = %d\n", a, b, top->f);
        assert(top->f == a ^ b);
    }
    // close the trace
    tfp->close();

    // Final model cleanup
    top->final();
    // Return good completion status
    // Don't use exit() or destructor won't get called
    return 0;
}
