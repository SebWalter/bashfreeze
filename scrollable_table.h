#ifndef SCROLLABLE_TABLE_H
#define SCROLLABLE_TABLE_H

#include "class_Process.h"
#include <memory>
#include <ncurses.h>
#include <vector>

WINDOW *init_new_window(int rows, int columns, int x, int y);
void draw_table(WINDOW *win, std::vector<std::unique_ptr<Process>> &processes, int selected_row, int start, int end);
class Scrollable_Table {
      private:
        WINDOW *win;
        std::vector<std::unique_ptr<Process>> processes;
        int rows;
        int columns;
        int start;
        int end;
        int selected;

      public:
        Scrollable_Table(int rows, int columns, int position_x, int position_y);
        Scrollable_Table(std::vector<std::unique_ptr<Process>> processes, int rows, int columns, int position_x,
                         int position_y);
        std::vector<std::unique_ptr<Process>> *get_processes();
        void print_window_name(const char *name);

        // update methods
        void set_processes(std::vector<std::unique_ptr<Process>> *processes);

        void set_table_dimensions(int rows, int columns);
        void update_window_dimensions(int x, int y);
	std::unique_ptr<Process> remove_process(int index_to_remove);

	void add_process(std::unique_ptr<Process> process_to_add);

        void update_window_dimensions(int rows, int columns, int x, int y);

        void set_selected(int selected);

	int get_selected();

        void selected_increment();

        void selected_decrement();

        void draw_table();
};

#endif // SCROLLABLE_TABLE_H
