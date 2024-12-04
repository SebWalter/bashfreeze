#include "class_Process.h"
#include <memory>
#ifndef PROCESSREADER_WORKER_H
#define PROCESSREADER_WORKER_H

using namespace std;

int init_worker();
int destroy_worker();
vector<unique_ptr<Process>> get_process_vector();
void request_process_vector();

#endif
