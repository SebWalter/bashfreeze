#ifndef SCROLLABLE_TABLE_H
#define SCROLLABLE_TABLE_H

#include "class_Process.h"
#include <memory>
#include <ncurses.h>
#include <vector>

class Scrollable_Table {
      private:
        WINDOW *win;
        std::vector<std::unique_ptr<Process>> processes;
        int rows;         // Total window rows
        int columns;      // Total window columns
        int display_rows; // Rows available for data
        int start;
        int end;
        int selected;

        static WINDOW *create_ncurses_window(int rows, int columns, int x, int y);
        void adjust_scroll_and_selection();

      public:

        // Static method to calculate required height
        static int get_required_total_height(int num_data_rows);

        Scrollable_Table(int total_rows, int total_columns, int position_x, int position_y);
        Scrollable_Table(std::vector<std::unique_ptr<Process>> processes, int total_rows, int total_columns,
                         int position_x, int position_y);
        const std::vector<std::unique_ptr<Process>> &get_processes() const;
        void print_window_name(const char *name);

        // update methods
        void set_processes(std::vector<std::unique_ptr<Process>> processes_to_set);

        void set_table_dimensions(int total_rows, int total_columns);
        void resize_and_relocate_window(int new_total_rows, int new_total_cols, int new_pos_x, int new_pos_y);
        std::unique_ptr<Process> remove_process(int index_to_remove);

        void add_process(std::unique_ptr<Process> process_to_add);

        void set_selected(int selected);

	int get_display_rows();

	void set_display_rows(int new_display_rows);

        int get_selected();

        void selected_increment();

        void selected_decrement();

        void draw_table();
	int get_process_count();
};

#endif
