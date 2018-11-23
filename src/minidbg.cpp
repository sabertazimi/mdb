#include <vector>
#include <sstream>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include "linenoise.h"

#include "breakpoint.hpp"
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

bool is_prefix(const std::string& s, const std::string& of) {
    if (s.size() > of.size()) return false;
    return std::equal(s.begin(), s.end(), of.begin());
}

void Debugger::handle_command(const std::string& line) {
    auto args = split(line,' ');

    if (args.empty()) {
        continue_execution();
    } else {
        auto command = args.front();
        if (is_prefix(command, "cont") || is_alias(command, "continue")) {
            continue_execution();
        } else if (is_prefix(command, "break") || is_alias(command, "break")) {
            std::string addr {args[1], 2}; // remove "0x" prefix
            set_breakpoint_at_address(std::stol(addr, 0, 16));
        } else {
            std::cerr << "Unknown command\n";
        }
    }
}

void Debugger::set_breakpoint_at_address(std::intptr_t addr) {
    std::cout << "Set breakpoint at address 0x" << std::hex << addr << std::endl;
    BreakPoint bp {m_pid, addr};
    bp.enable();
    m_breakpoints[addr] = bp;
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

void Debugger::continue_execution() {
    ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);
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
