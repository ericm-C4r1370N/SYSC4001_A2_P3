SYSC4001_A2_P3
Contents:

    interrupts.cpp: CPU simulation program
    interrupts.hpp: helper functions for simulator
    trace.txt: program execution trace
    device_table.txt: table of hardcoded IO times for corresponding device #'s
    vector_table.txt: table of memory addressses containing ISR locations for each IO device.
    external_files.txt: table of external files containing additional programs
    program#N: where n is the program # of additional external programs

Supported Platforms:

    x86-64 based linux

Build dependencies:

    gcc/g++

Usage (scripted):

    clone repo
    cd into repo
    run "source build.sh"
    program will be compiled to ./bin/interrupts
    program will execute and output to ./execution.txt
    optional: interpret results with additional program

Usage (manual):

    clone repo
    cd into repo
    compile interrupts.cpp with interrupts.hpp using your preferred C/C++ compiler
    run compiled binary
    optional: interpret results with additional program
