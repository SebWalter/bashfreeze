#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sched.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <filesystem>
#include "processreader.h"
using namespace std;
namespace fs = std::filesystem;
//converts a string into a pid_t
static pid_t string_to_pid(string s){
	int i = stoi(s);
	return static_cast<pid_t>(i);

}
/*opens the file /proc/ and reads all process_ids
 * and stores them into a vector<pid_t> and returns them
 */
static map<pid_t, bool> get_all_process_IDs(){
	map<pid_t, bool> process_ids;	
	try {
	//iterates over all directories in the folder "/proc/"
	for(const fs::directory_entry entry : fs::directory_iterator("/proc/")) {
		//if entry is no folder -> irgnore it
		if(fs::is_directory(entry.status())) {
			//not all folders are numbers (not able to convert them to pid_t's
			//--> try to convert them
			//in case it fails -> ignore
			try{
				string subfolder_name = entry.path().filename();
				pid_t new_pid = string_to_pid(subfolder_name);
				process_ids[new_pid] = true;
			}catch(exception& e){
				continue;
			}
		}

	}
	//iterating fails: problematic error
	} catch (exception& e) {
		cerr<<"Failed to open proc file: "<<e.what()<<endl;
		exit(EXIT_FAILURE);
	}
	return process_ids;
}

static vector<pid_t> map_to_vector(map<pid_t, bool> pids) {
	vector<pid_t> pids_vector;
	for (auto const& [key, value] : pids) {
		pids_vector.push_back(key);
	}
	return pids_vector;

}

//test funciton to test functionalities
static unique_ptr<Process> create_processes_structure(map<pid_t, bool> pids) {
	vector<pid_t> pids_vector = map_to_vector(pids);
	unique_ptr<Process> main_processes;

	for(int i = 0; i < pids_vector.size(); i++) {
		pid_t current_pid = pids_vector[i];
		
	




}
int main() {
	vector<pid_t> process_ids = get_all_process_IDs();
	for (int i = 0; i < process_ids.size(); i++){
		cout<<process_ids[i]<<endl;
	}
}
