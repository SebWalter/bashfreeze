#include "class_Process.h"
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>
using namespace std;
namespace fs = std::filesystem;
// converts a string into a pid_t
static pid_t string_to_pid(string s) {
        int i = stoi(s);
        return static_cast<pid_t>(i);
}
/*opens the file /proc/ and reads all process_ids
 * and stores them into a vector<pid_t> and returns them
 */
static vector<pid_t> get_all_process_IDs() {
        vector<pid_t> process_ids;
        try {
                // iterates over all directories in the folder "/proc/"
                for (const fs::directory_entry entry : fs::directory_iterator("/proc/")) {
                        // if entry is no folder -> irgnore it
                        if (fs::is_directory(entry.status())) {
                                // not all folders are numbers (not able to convert them to pid_t's
                                //--> try to convert them
                                // in case it fails -> ignore
                                try {
                                        string subfolder_name = entry.path().filename();
                                        pid_t new_pid = string_to_pid(subfolder_name);
                                        process_ids.push_back(new_pid);
                                } catch (exception &e) {
   continue;
                                }
                        }
                }
                // iterating fails: problematic error
        } catch (exception &e) {
                cerr << "Failed to open proc file: " << e.what() << endl;
		throw e;
        }
        return process_ids;
}
/*this function return the group id found in the file /proc/pid_t/stat
 * if it fails to open the file, it throws an exception
 */
static pid_t getGroupID(pid_t pid_to_check) {
        pid_t groupID = -1;
        string pid_string = to_string(pid_to_check);
        // creates the full path name
        string path = "/proc/" + pid_string + "/stat";
        // opens the stream
        ifstream stat_file(path);
        // if the file fails to open -> throw exception
        if (stat_file.is_open()) {
                string element;
                // the 5 element of the stat file is the group id. -> element_count == 4 -> group id as a string
                int element_count = 0;
                // read every word from that file until the 5 or eof;
                while (stat_file >> element) {
                        if (element_count == 1) {
                                if (element.find(")") == string::npos) {
                                        continue;
                                }
                        }
                        if (element_count == 4) {
                                groupID = string_to_pid(element);
                                break;
                        }
                        element_count++;
                }
                stat_file.close();
        } else {
                throw ifstream::failure("Could not open file");
        }
        // if the groupPID is still -1 something wrent wrong
        if (groupID == -1) {
                throw invalid_argument("Negative goupPID");
        }
        return groupID;
}

// test funciton to test functionalities
static vector<unique_ptr<Process>> create_processes_structure(vector<pid_t> pids_vector) {
        map<pid_t, Process> map_processes;
        pid_t group_id;
        // iterate over all elements in pids_vector, contains all pids found in /proc/
        for (int i = 0; i < pids_vector.size(); i++) {
                pid_t current_pid = pids_vector[i];
                try {
                        // get the group_ID
                        group_id = getGroupID(current_pid);
                        // if the group_id failed, something in the called function wrent horribly wrong
                        // All processes have a group_id, when it isn't in a group the id is the same ass group_id
                } catch (exception &e) {
                        cerr << "Failed to find the group ID of: " << current_pid << "due :" << e.what() << endl;
			throw e;
                }
                // group_ids are the main processes, so we categorize all processes after them
                // If a group_id is not in the map, we have to insert it
                if (map_processes.count(group_id) < 1) {
                        Process new_main_process = Process(group_id);
                        // when group_id != current_pid than it is a childprocess. -> append it to the child_process
                        // vector
                        if (current_pid != group_id) {
                                new_main_process.get_child_process()->push_back(current_pid);
                        }
                        map_processes[group_id] = new_main_process;
                }
                // when the group_id is already in the map ->check if it is not a main process -> append it to the
                // main_process child list
                else if (group_id != current_pid) {
                        map_processes[group_id].get_child_process()->push_back(current_pid);
                }
        }
        // turns the map into a vector with unique ptr's
        vector<unique_ptr<Process>> main_processes;
        for (auto const &[group_id, current_process] : map_processes) {
                main_processes.push_back(make_unique<Process>(current_process));
        }
        return main_processes;
}
static void get_all_names(const vector<unique_ptr<Process>> &main_processes) {
        for (int i = 0; i < main_processes.size(); i++) {
                if (main_processes[i]->get_process_id() == 0) {
                        continue;
                }
                main_processes[i]->find_name();
        }
        return;
}
vector<unique_ptr<Process>> generate_process_vector() {
        vector<pid_t> all_pids = get_all_process_IDs();
        vector<unique_ptr<Process>> main_processes = create_processes_structure(all_pids);
        get_all_names(main_processes);
        return std::move(main_processes);
}
/*
int main() {
        vector<unique_ptr<Process>> main_processes = generate_process_vector();
        for (int i = 0; i < main_processes.size(); i++){
                cout<<main_processes[i]->get_name()<<": "<<main_processes[i]->get_process_id()<<endl;
                for(int k = 0; k < main_processes[i]->get_child_process()->size(); k++) {
                        cout<<"|-->"<<(int)main_processes[i]->get_child_pid(k)<<endl;
                }
        }
}
*/
