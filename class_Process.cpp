#include "class_Process.h"
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

void Process::find_name() {
        std::string pid_string = std::to_string(this->process_pid);
        std::string path = "/proc/" + pid_string + "/stat";
        
        // Check if process directory exists
        if (!std::filesystem::exists("/proc/" + pid_string)) {
                return;
        }
        
        std::ifstream stat_file(path);
        if (!stat_file.is_open()) {
                return;  // Silently skip if we can't open the file
        }
        
        std::string name = "";
        char c;
        bool name_found = false;
        while (stat_file.get(c)) {
                if (!name_found) {
                        if (c == '(') {
                                name_found = true;
                        }
                } else {
                        if (c == ')') {
                                this->process_name = name;
                                break;
                        }
                        name += c;
                }
        }
        stat_file.close();
        return;
}
