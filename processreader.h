#ifndef PROCESSREADER_H
#define PROCESSREADER_H
#include <memory>
#include <sched.h>
#include <string>
#include <sys/types.h>
#include <vector>
class Process {
private:
	pid_t process_pid;
	std::string process_name;
	std::vector<pid_t> child_processes;
public:
	Process() {}
	Process(pid_t pid){
		this->process_pid = pid;
	}
	pid_t get_process_id() const {
		return this->process_pid;
	}
	std::string get_name() const {
		return this->get_name();
	}
	std::vector<pid_t> * get_child_process(){
		return &this->child_processes;
	}
	pid_t get_child_pid(int index) {
		return this->child_processes[index];
	}
	void set_name(std::string new_name){
		this->process_name = new_name;
	}
	};




#endif
