#ifndef SCROLLABLE_TABLE_H
#define SCROLLABLE_TABLE_H

#include <memory>
#include <ncurses.h>
#include <vector>

class Process; // Forward declaration for Process class

WINDOW *init_new_window(int rows, int columns, int x, int y);
void draw_table(WINDOW *win, std::vector<std::unique_ptr<Process>> &processes, int selected_row, int start, int end);

class Scrollable_Table {
      public:
        Scrollable_Table(int rows, int columns, int position_x, int position_y);
        Scrollable_Table(std::vector<std::unique_ptr<Process>> processes, int rows, int columns, int position_x,
                         int position_y);
        std::vector<std::unique_ptr<Process>> *get_processes();
        void print_window_name(const char *name);

        // update methods
        void set_processes(std::vector<std::unique_ptr<Process>> processes);

        void set_table_dimensions(int rows, int columns);
        void update_window_dimensions(int x, int y);

        void update_window_dimensions(int rows, int columns, int x, int y);

        void set_selected(int selected);

        void selected_increment();

        void selected_decement();

        int get_input();
        void draw_table();

      private:
        WINDOW *win;
        std::vector<std::unique_ptr<Process>> processes;
        int rows;
        int columns;
        int start;
        int end;
        int selected;
};

#endif // SCROLLABLE_TABLE_H
