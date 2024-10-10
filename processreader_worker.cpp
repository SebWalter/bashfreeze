#include "class_Process.h"
#include "processreader.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
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
//worker process, that creates the process vector, if the sem ist incremented
//dies when the bool worker_die is set to true -> happens in the die_worker function
static void worker_function() {
	while(!worker_die) {
		//only gets passt the sem if request_new_vector is called, or worker_die
		sem_wait(&worker_sem);
		//checks if worker should die
		if (worker_die) {
			break;
		}
		
		vector<unique_ptr<Process>> new_main_processes = generate_process_vector();
		//save access to stack
		sem_wait(&vector_sem);
		communication_stack.push((std::move(new_main_processes)));
		sem_post(&vector_sem);
		sem_post(&worker_sem);
	}



	return;

}
//get's the first vector from the stack, or throws exception
vector<unique_ptr<Process>> get_process_vector() {
	if (communication_stack.empty()) {
		throw runtime_error("No new Process vector");
	}
	sem_wait(&vector_sem);
	vector<unique_ptr<Process>> top_Process_vector = std::move(communication_stack.top());
	communication_stack.pop();
	sem_post(&vector_sem);
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
		thread worker(worker_function);
	}catch(exception e){
		cerr<<"Failed to initialize worker thread: "<<e.what()<<endl;
		return -1;
	}
	return 0;
}
