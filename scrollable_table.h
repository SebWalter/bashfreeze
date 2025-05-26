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
        static const int BORDER_WIDTH = 1; // Width of the border on each side (e.g., for box(win,0,0))
        // static const int HEADER_HEIGHT = 1; // Height of the header text line itself. Replaced by
        // COLUMN_HEADER_OFFSET_Y
        static const int WINDOW_FRAME_VERTICAL_PADDING = 2; // Total lines used by top and bottom window borders
        static const int HEADER_AREA_HEIGHT = 2; // Total lines used by title and column headers area (above data)

        // New constants for drawing layout:
        static const int COLUMN_HEADER_OFFSET_Y = 1;  // Y-offset from window top for "PID", "Name" headers
        static const int SEPARATOR_LINE_OFFSET_Y = 2; // Y-offset for the horizontal line below headers
        static const int DATA_START_OFFSET_Y = 3;     // Y-offset for the first line of data
        static const int INTER_COLUMN_SPACING = 1;    // Horizontal space between columns
        static const int NUMBER_OF_COLUMNS = 2;       // Number of columns (PID, Name)

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

        int get_selected();

        void selected_increment();

        void selected_decrement();

        void draw_table();
};

#endif
