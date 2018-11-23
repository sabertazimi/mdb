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

        void dump_registers();
        void set_breakpoint_at_address(std::intptr_t addr);
        void run();

    private:
        inline void init(void) {
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

        inline bool is_alias(const std::string& input, const std::string& command) {
            auto alias = m_aliases.find(input);
            return (alias != m_aliases.end() && alias->second == command);
        }

        inline void set_alias(const std::string& input, const std::string& command) {
            m_aliases[input] = command;
        }

        void handle_command(const std::string& line);
        void continue_execution();        
        
        std::string m_prog_name;
        pid_t m_pid;
        std::unordered_map<std::string, std::string> m_aliases;
        std::unordered_map<std::intptr_t, BreakPoint> m_breakpoints;
    };
}

#endif
