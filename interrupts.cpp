/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

std::tuple<std::string, std::string, int> simulate_trace(
    std::vector<std::string> trace_file, int time,
    std::vector<std::string> vectors, std::vector<int> delays,
    std::vector<external_file> external_files, PCB current,
    std::vector<PCB> wait_queue
) {
    std::string execution = "";
    std::string system_status = "";
    int current_time = time;

    for (size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];
        auto [activity, duration_intr, program_name] = parse_trace(trace);

        if (activity == "CPU") {
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;

        } else if (activity == "SYSCALL") {
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr;
            current_time = time;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", SYSCALL ISR executed\n";
            current_time += delays[duration_intr];

            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

        } else if (activity == "END_IO") {
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr;
            current_time = time;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", END_IO ISR executed\n";
            current_time += delays[duration_intr];

            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

        } else if (activity == "FORK") {
            auto [intr, time] = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr;
            current_time = time;

            static unsigned int next_pid = 1;
            unsigned int child_pid = next_pid++;
            PCB child(child_pid, current.PID, current.program_name + "_child", current.size, -1);

            if (!allocate_memory(&child)) {
                execution += std::to_string(current_time) + ", 1, Memory full, child process cannot be created\n";
            } else {
                execution += std::to_string(current_time) + ", 2, Child process created (PID " + std::to_string(child.PID) + ")\n";
            }

            wait_queue.push_back(child);
            system_status += print_PCB(current, wait_queue);

            // Child trace
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_flag = false;
            int parent_index = 0;

            for (size_t j = i; j < trace_file.size(); j++) {
                auto [_activity, _duration, _pn] = parse_trace(trace_file[j]);
                if (skip && _activity == "IF_CHILD") { skip = false; continue; }
                else if (_activity == "IF_PARENT") { skip = true; parent_index = j; if(exec_flag) break; }
                else if (skip && _activity == "ENDIF") { skip = false; continue; }
                else if (!skip && _activity == "EXEC") { skip = true; child_trace.push_back(trace_file[j]); exec_flag = true; }
                if (!skip) child_trace.push_back(trace_file[j]);
            }
            i = parent_index;

            // Run child (with recursion)
            if (child.partition_number != -1) {
                auto [child_exec, child_status, new_time] = simulate_trace(
                    child_trace, current_time, vectors, delays, external_files, child, {}
                );
                execution += child_exec;
                system_status += child_status;
                current_time = new_time;
                free_memory(&child);
            }

            execution += std::to_string(current_time) + ", 1, Child terminated, returning to parent\n";
            current_time++;
            wait_queue.pop_back();

        } else if (activity == "EXEC") {
            auto [intr, time] = intr_boilerplate(current_time, 3, 10, vectors);
            execution += intr;
            current_time = time;

            unsigned int new_size = get_size(program_name, external_files);
            free_memory(&current);
            current.program_name = program_name;
            current.size = new_size;

            if (!allocate_memory(&current)) {
                execution += std::to_string(current_time) + ", 1, EXEC failed (no space)\n";
            } else {
                execution += std::to_string(current_time) + ", 2, EXEC successful, running " + program_name + "\n";
                // simulated memory load time = size * 15 
                int load_time = current.size * 15; 
                execution += std::to_string(current_time + 1) + ", " + std::to_string(load_time) + ", loading program into memory\n";
                current_time += load_time + 1;
            }

            system_status += print_PCB(current, wait_queue);

            std::ifstream exec_trace_file(program_name + ".txt");
            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while (std::getline(exec_trace_file, exec_trace)) exec_traces.push_back(exec_trace);

            auto [exec_out, exec_status, new_time] = simulate_trace(
                exec_traces, current_time, vectors, delays, external_files, current, wait_queue
            );
            execution += exec_out;
            system_status += exec_status;
            current_time = new_time;

            break; // important break - highlight in report
        }
    }

    return {execution, system_status, current_time};
}


int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //external_files is a C++ std::vector of the struct 'external_file'. Check the struct in 
    //interrupt.hpp to know more.
    auto [vectors, delays, external_files] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    //Just a sanity check to know what files you have
    print_external_files(external_files);

    //Make initial PCB (notice how partition is not assigned yet)
    PCB current(0, -1, "init", 1, -1);
    //Update memory (partition is assigned here, you must implement this function)
    if(!allocate_memory(&current)) {
        std::cerr << "ERROR! Memory allocation failed!" << std::endl;
    }

    std::vector<PCB> wait_queue;

    /******************ADD YOUR VARIABLES HERE*************************/
    /******************************************************************/

    //Converting the trace file into a vector of strings.
    std::vector<std::string> trace_file;
    std::string trace;
    while(std::getline(input_file, trace)) {
        trace_file.push_back(trace);
    }

    auto [execution, system_status, _] = simulate_trace(   trace_file, 
                                            0, 
                                            vectors, 
                                            delays,
                                            external_files, 
                                            current, 
                                            wait_queue);

    input_file.close();

    write_output(execution, "execution.txt");
    write_output(system_status, "system_status.txt");

    return 0;
}
