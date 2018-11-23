#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "linenoise.h"

#include "breakpoint.hpp"
#include "registers.hpp"
#include "debugger.h"

using namespace minidbg;

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> out{};
    std::stringstream ss {s};
    std::string item;

    while (std::getline(ss,item,delimiter)) {
        out.push_back(item);
    }

    return out;
}

void Debugger::run() {
    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);

    char* line = nullptr;
    while ((line = linenoise("minidbg> ")) != nullptr) {
        handle_command(line);
        linenoiseHistoryAdd(line);
        linenoiseFree(line);
    }
}

void Debugger::handle_command(const std::string& line) {
    auto args = split(line,' ');

    if (args.empty()) {
        continue_execution();
    } else {
        auto command = args.front();
        if (is_alias(command, "continue")) {
            continue_execution();
        } else if (is_alias(command, "break")) {
            std::string addr {args[1], 2}; // remove "0x" prefix
            set_breakpoint_at_address(std::stol(addr, 0, 16));
        } else if (is_alias(command, "register")) {
            if (is_alias(args[1], "dump")) {
                dump_registers();
            } else if (is_alias(args[1], "read")) {
                std::cout << get_register_value(m_pid, get_register_from_name(args[2])) << std::endl;
            } else if (is_alias(args[1], "write")) {
                std::string val {args[3], 2}; //assume 0xVAL
                set_register_value(m_pid, get_register_from_name(args[2]), std::stol(val, 0, 16));
            }
        } else {
            std::cerr << "Unknown command\n";
        }
    }
}

void Debugger::continue_execution() {
    ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);
}

void Debugger::set_breakpoint_at_address(std::intptr_t addr) {
    std::cout << "Set breakpoint at address 0x" << std::hex << addr << std::endl;
    BreakPoint bp {m_pid, addr};
    bp.enable();
    m_breakpoints[addr] = bp;
}

void Debugger::dump_registers() {
    for (const auto& rd : g_register_descriptors) {
        std::cout << rd.name << " 0x"
                  << std::setfill('0') << std::setw(16) << std::hex
                  << get_register_value(m_pid, rd.r) << std::endl;
    }
}

inline void Debugger::init(void) {
    this->set_alias("c", "continue");
    this->set_alias("cont", "continue");
    this->set_alias("continue", "continue");

    this->set_alias("b", "break");
    this->set_alias("break", "break");

    this->set_alias("reg", "register");
    this->set_alias("register", "register");

    this->set_alias("d", "dump");
    this->set_alias("dump", "dump");
    this->set_alias("r", "read");
    this->set_alias("read", "read");
    this->set_alias("w", "write");
    this->set_alias("write", "write");
}

inline bool Debugger::is_alias(const std::string& input, const std::string& command) {
    auto alias = m_aliases.find(input);
    return (alias != m_aliases.end() && alias->second == command);
}

inline void Debugger::set_alias(const std::string& input, const std::string& command) {
    m_aliases[input] = command;
}

void execute_debugee (const std::string& prog_name) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        std::cerr << "Error in ptrace\n";
        return;
    }

    execl(prog_name.c_str(), prog_name.c_str(), nullptr);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Program name not specified";
        return -1;
    }

    auto prog = argv[1];

    auto pid = fork();
    if (pid == 0) {
        // child
        execute_debugee(prog);
    } else if (pid >= 1) {
        // parent
        Debugger dbg{prog, pid};
        dbg.run();
    }
}
