#ifndef _MINIDBG_DEBUGGER_H
#define _MINIDBG_DEBUGGER_H

#include <utility>
#include <string>
#include <unordered_map>
#include <linux/types.h>
#include <breakpoint.hpp>

namespace minidbg {
    class Debugger {
    public:
        Debugger (std::string prog_name, pid_t pid)
            : m_prog_name{std::move(prog_name)}, m_pid{pid} {
            this->init();
        }

        void run();

    private:
        void handle_command(const std::string& line);

        void continue_execution();        
        void set_breakpoint_at_address(std::intptr_t addr);
        void dump_registers();

        inline void init(void);
        inline auto is_alias(const std::string& input, const std::string& command) -> bool;
        inline void set_alias(const std::string& input, const std::string& command);
        
        std::string m_prog_name;
        pid_t m_pid;
        std::unordered_map<std::string, std::string> m_aliases;
        std::unordered_map<std::intptr_t, BreakPoint> m_breakpoints;
    };
}

#endif
