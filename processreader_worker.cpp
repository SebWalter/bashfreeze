#include "class_Process.h"
#include "processreader.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <pthread.h>
#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>
#include <semaphore.h>
#include <thread>
using namespace std;


static stack<vector<unique_ptr<Process>>> communication_stack;

static sem_t vector_sem;
static sem_t worker_sem;
static bool worker_die = false;
static thread worker;
//worker process, that creates the process vector, if the sem ist incremented
//dies when the bool worker_die is set to true -> happens in the die_worker function
static void worker_function() {
	while(!worker_die) {
		//only gets passt the sem if request_new_vector is called, or worker_die
		if (sem_wait(&worker_sem) != 0) {
			cerr<<"Failed to wait for sem: "<<strerror(errno)<<endl;
			exit(EXIT_FAILURE);
		}
		//checks if worker should die
		if (worker_die) {
			break;
		}
		
		vector<unique_ptr<Process>> new_main_processes = generate_process_vector();
		//save access to stack
		if (sem_wait(&vector_sem) != 0) {
			cerr<<"Failed to wait for sem: "<<strerror(errno)<<endl;
			exit(EXIT_FAILURE);
		}
		communication_stack.push((std::move(new_main_processes)));
		if (sem_post(&vector_sem) != 0) {
			cerr<<"Failed to post sem: "<<strerror(errno)<<endl;
			exit(EXIT_FAILURE);
		}
		if (sem_post(&worker_sem) != 0) {
			cerr<<"Failed to post sem: "<<strerror(errno)<<endl;
			exit(EXIT_FAILURE);
		}
	}



	return;

}
void request_process_vector() {
	if (sem_post(&worker_sem) != 0) {
		cerr<<"Failed to post sem: "<<strerror(errno)<<endl;
		exit(EXIT_FAILURE);
	}
	return;
}

//get's the first vector from the stack, or throws exception
vector<unique_ptr<Process>> get_process_vector() {
	if (communication_stack.empty()) {
		throw runtime_error("No new Process vector");
	}
	if (sem_wait(&vector_sem) != 0) {
		cerr<<"Failed to wait for sem: "<<strerror(errno)<<endl;
		exit(EXIT_FAILURE);
	}
	vector<unique_ptr<Process>> top_Process_vector = std::move(communication_stack.top());
	communication_stack.pop();
	if (sem_post(&vector_sem) != 0) {
		cerr<<"Failed to post sem: "<<strerror(errno)<<endl;
		exit(EXIT_FAILURE);
	}
	return std::move(top_Process_vector);
}

//init's worker and starts new thread
int init_worker(){
	if (sem_init(&vector_sem, 0, 1) < 0){
		cerr<<"Failed to initialize semaphore: "<<strerror(errno)<<endl;
		return -1;
	}
	if (sem_init(&worker_sem, 0, 0) < 0) {
		cerr<<"Failed to initialize semaphore: "<<strerror(errno)<<endl;
		return -1;
	}
	try {
		worker = thread(worker_function);
	}catch(exception e){
		cerr<<"Failed to initialize worker thread: "<<e.what()<<endl;
		return -1;
	}
	return 0;
}
int destroy_worker(){
	if (sem_close(&vector_sem) != 0) {
		cerr<<"Failed to close semaphore: "<<strerror(errno)<<endl;
		return -1;
	}
	worker_die = true;
	if (sem_post(&worker_sem) != 0) {
		cerr<<"Failed to increase semaphore:: "<<strerror(errno)<<endl;
		return -1;
	}
	worker.join();
	if (sem_close(&worker_sem) != 0) {
		cerr<<"Failed to close semaphore: "<<strerror(errno)<<endl;
		return -1;
	}
	return 0;
}


