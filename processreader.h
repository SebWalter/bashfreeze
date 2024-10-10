#ifndef PROCESSREADER_H
#define PROCESSREADER_H
#include <memory>
#include <vector>
#include "class_Process.h"

std::vector<std::unique_ptr<Process>> generate_process_vector();


#endif	
