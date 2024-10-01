#include <cstring>
#include <exception>
#include <iostream>
#include <ostream>
#include <sched.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;
static pid_t string_to_pid(string s){
	int i = stoi(s);
	return static_cast<pid_t>(i);

}

static vector<pid_t> get_all_process_IDs(){
	vector<pid_t> process_ids;	
	try {
	for(const fs::directory_entry entry : fs::directory_iterator("/proc/")) {
		if(fs::is_directory(entry.status())) {
			try{
				string subfolder_name = entry.path().filename();
				pid_t new_pid = string_to_pid(subfolder_name);
				process_ids.push_back(new_pid);
			}catch(exception& e){
				continue;
			}
		}

	}
	} catch (exception& e) {
		cerr<<"Failed to open proc file: "<<e.what()<<endl;
	
	}

	return process_ids;
}
int main() {
	vector<pid_t> process_ids = get_all_process_IDs();
	for (int i = 0; i < process_ids.size(); i++){
		cout<<process_ids[i]<<endl;
	}
}
