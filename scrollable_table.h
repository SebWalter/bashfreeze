#ifndef SCROLLABLE_TABLE_H
#define SCROLLABLE_TABLE_H
#include <ncurses.h>
#include "class_Process.h"
#include <vector>

using namespace std;
void draw_table(WINDOW *win, vector<unique_ptr<Process>> &processes, int selected_row, int start, int end);


#endif
