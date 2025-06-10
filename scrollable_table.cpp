#include "scrollable_table.h"
#include "class_Process.h"
// #include "processreader.h" // Removed unused include
#include <cmath>
#include <cstdio> // For snprintf
#include <memory>
#include <ncurses.h>
#include <stdlib.h> // For general utilities, often included with ncurses
#include <string>
#include <utility> // For std::move
#include <vector>

#define BORDER_WIDTH 1 // Width of the border on each side (e.g., for box(win,0,0))
// static const int HEADER_HEIGHT = 1; // Height of the header text line itself. Replaced by
// COLUMN_HEADER_OFFSET_Y
#define WINDOW_FRAME_VERTICAL_PADDING 2 // Total lines used by top and bottom window borders
#define HEADER_AREA_HEIGHT 2 // Total lines used by title and column headers area (above data)

// New constants for drawing layout:
#define COLUMN_HEADER_OFFSET_Y 1  // Y-offset from window top for "PID", "Name" headers
#define SEPARATOR_LINE_OFFSET_Y 2 // Y-offset for the horizontal line below headers
#define DATA_START_OFFSET_Y 3     // Y-offset for the first line of data
#define INTER_COLUMN_SPACING 1    // Horizontal space between columns
#define NUMBER_OF_COLUMNS 2       // Number of columns (PID, Name)

static int max_process_display_count = 0;

using namespace std;


WINDOW *Scrollable_Table::create_ncurses_window(int rows, int columns, int x, int y) {
        if (rows <= 0 || columns <= 0) {
                return nullptr;
        }
        WINDOW *win = newwin(rows, columns, x, y);
        if (win) {
                box(win, 0, 0);
                keypad(win, true);
                wtimeout(win, 6000);
        }
        return win;
}

Scrollable_Table::Scrollable_Table(int total_rows, int total_columns, int position_x, int position_y) {
        this->win = create_ncurses_window(total_rows, total_columns, position_x, position_y);
        this->rows = total_rows;
        this->columns = total_columns;
        this->display_rows = this->rows - WINDOW_FRAME_VERTICAL_PADDING - DATA_START_OFFSET_Y;
        if (this->display_rows < 0)
                this->display_rows = 0;

        this->start = 0;
        // this->end = this->start + this->display_rows; // end will be set by adjust_scroll_and_selection
        this->selected = 0;
        adjust_scroll_and_selection();
}

Scrollable_Table::Scrollable_Table(vector<unique_ptr<Process>> processes, int total_rows, int total_columns,
                                   int position_x, int position_y) {
        this->processes = std::move(processes);
        this->win = create_ncurses_window(total_rows, total_columns, position_x, position_y);
        this->rows = total_rows;
        this->columns = total_columns;
        // Corrected display_rows calculation
        this->display_rows = this->rows - WINDOW_FRAME_VERTICAL_PADDING - DATA_START_OFFSET_Y;
        if (this->display_rows < 0)
                this->display_rows = 0;

        this->start = 0;
        // this->end = this->start + this->display_rows; // end will be set by adjust_scroll_and_selection
        this->selected = 0;
        adjust_scroll_and_selection();
}

// todo: write a function that calls the calculator of max lines above
// compares how much processes are in each table schaut dann es sich bei 50/50 einpendelt, 
// ansonsten zeigt mehr in der einne tabelle an
void Scrollable_Table::adjust_scroll_and_selection() {
        // Clamp selected to be within [0, processes.size() - 1]
        if (processes.empty()) {
                selected = 0;
        } else {
                if (selected >= processes.size()) {
                        selected = processes.size() - 1;
                }
                if (selected < 0) {
                        selected = 0;
                }
        }

        if (this->display_rows <= 0) {
                start = 0;
                end = 0;
                selected = 0;
                return;
        }

        // Adjust start and end based on selected and processes.size()
        if (selected >= start + this->display_rows) { // If selected is below the current view
                start = selected - this->display_rows + 1;
        } else if (selected < start) { // If selected is above the current view
                start = selected;
        }

        // Ensure start is not negative
        if (start < 0) {
                start = 0;
        }

        end = start + this->display_rows;
}

int Scrollable_Table::get_required_total_height(int num_data_rows) {
        if (num_data_rows < 0)
                num_data_rows = 0;
        // Total height = data rows + lines for title/header/separator + lines for top/bottom box border
        return num_data_rows + DATA_START_OFFSET_Y + WINDOW_FRAME_VERTICAL_PADDING;
}

void Scrollable_Table::set_display_rows(int new_display_rows) {
	this->display_rows = new_display_rows;
}
int Scrollable_Table::get_display_rows(){
	return this->display_rows;
}

const std::vector<std::unique_ptr<Process>> &Scrollable_Table::get_processes() const {
        return this->processes;
}

void Scrollable_Table::print_window_name(const char *name) {
        if (this->win == NULL) {
                return;
        }
        mvwprintw(this->win, 0, 2, " %s ", name);
        return;
}

void Scrollable_Table::set_processes(std::vector<std::unique_ptr<Process>> processes_to_set) {
        this->processes = std::move(processes_to_set);
        this->adjust_scroll_and_selection();
}

unique_ptr<Process> Scrollable_Table::remove_process(int index_to_remove) {
        int vecSize = this->processes.size();
        if (vecSize <= 0 || index_to_remove < 0 ||
            index_to_remove >= vecSize) { // Check for index_to_remove < 0 was from prev step
                return NULL;
        }
        unique_ptr<Process> removed_process = std::move(this->processes[index_to_remove]);
        this->processes.erase(this->processes.begin() + index_to_remove);
        adjust_scroll_and_selection(); // Call to adjust_scroll_and_selection was from prev step
        return removed_process;
}
void Scrollable_Table::add_process(std::unique_ptr<Process> process_to_add) {
        if (process_to_add) { // Check for valid pointer was from prev step
                this->processes.push_back(std::move(process_to_add));
                adjust_scroll_and_selection(); // Call to adjust_scroll_and_selection was from prev step
        }
        return;
}

void Scrollable_Table::set_table_dimensions(int total_rows, int total_columns) {
        this->rows = total_rows;
        this->columns = total_columns;

        if (this->win) { // Ensure window exists before resizing
                wresize(this->win, this->rows, this->columns);
        } else {
        }

        this->display_rows = this->rows - WINDOW_FRAME_VERTICAL_PADDING - DATA_START_OFFSET_Y;
        if (this->display_rows < 0) {
                this->display_rows = 0;
        }

        adjust_scroll_and_selection();
}

void Scrollable_Table::resize_and_relocate_window(int new_total_rows, int new_total_cols, int new_pos_x,
                                                  int new_pos_y) {
        if (this->win != NULL) {
                delwin(this->win);
        }
        this->win = create_ncurses_window(new_total_rows, new_total_cols, new_pos_x, new_pos_y);
        this->rows = new_total_rows;
        this->columns = new_total_cols;

        this->display_rows = this->rows - WINDOW_FRAME_VERTICAL_PADDING - DATA_START_OFFSET_Y;
        if (this->display_rows < 0)
                this->display_rows = 0;

        adjust_scroll_and_selection();
}

void Scrollable_Table::set_selected(int val) { // Renamed parameter to avoid confusion with member
        if (val < 0) {
                this->selected = 0;
        } else if (!processes.empty() && val >= processes.size()) {
                this->selected = processes.size() - 1;
        } else if (processes.empty()) {
                this->selected = 0;
        } else {
                this->selected = val;
        }
        adjust_scroll_and_selection(); // Ensure view updates based on new selection
}

int Scrollable_Table::get_selected() {
        return this->selected;
}

void Scrollable_Table::selected_increment() {
        if (processes.empty()) {
                selected = 0; // Should already be, but for safety
                adjust_scroll_and_selection();
                return;
        }
        if (this->selected < this->processes.size() - 1) {
                this->selected++;
                adjust_scroll_and_selection();
        }
}

void Scrollable_Table::selected_decrement() {
        if (processes.empty()) {
                selected = 0; // Should already be, but for safety
                adjust_scroll_and_selection();
                return;
        }
        if (this->selected > 0) {
                this->selected--;
                adjust_scroll_and_selection();
        }
}
int Scrollable_Table::get_process_count() {
	return this->processes.size();
}

void Scrollable_Table::draw_table() {
        if (this->win == NULL) {
                return;
        }

        // Clear window content before redrawing to prevent artifacts
        werase(this->win);

        // Column width calculation
        // Usable width is total columns minus borders on both sides, minus spacing between columns
        int usable_content_width = this->columns - (2 * BORDER_WIDTH) - INTER_COLUMN_SPACING;
        if (usable_content_width < 0)
                usable_content_width = 0; // Prevent negative widths

        int pid_column_width = usable_content_width / 3;
        // Name column takes the rest of the space. Can be 0 if window is too small.
        int name_column_width = usable_content_width - pid_column_width;
        if (name_column_width < 0)
                name_column_width = 0;

        // Draw Headers
        // PID Header
        mvwprintw(win, COLUMN_HEADER_OFFSET_Y, BORDER_WIDTH, "PID");
        // Name Header
        mvwprintw(win, COLUMN_HEADER_OFFSET_Y, BORDER_WIDTH + pid_column_width + INTER_COLUMN_SPACING, "Name");

        // Separator Line
        mvwhline(win, SEPARATOR_LINE_OFFSET_Y, BORDER_WIDTH, ACS_HLINE, this->columns - (2 * BORDER_WIDTH));

        // Draw rows
        for (int i = start; i < end && i < processes.size(); i++) {
                if (i == this->selected) {
                        wattron(win, A_REVERSE);
                }

                int data_row_y = DATA_START_OFFSET_Y + (i - start);

                // PID Data - Ensure it fits within pid_column_width
                char pid_str[20]; // Buffer for PID string
                snprintf(pid_str, sizeof(pid_str), "%d", processes[i]->get_process_id());
                mvwprintw(win, data_row_y, BORDER_WIDTH, "%.*s", pid_column_width, pid_str);

                // Name Data - Ensure it fits within name_column_width
                const std::string &name_str_full = processes[i]->get_name();
                mvwprintw(win, data_row_y, BORDER_WIDTH + pid_column_width + INTER_COLUMN_SPACING, "%.*s",
                          name_column_width, name_str_full.c_str());

                if (i == this->selected) { // Turn off attribute for the rest of the line if selected
                        wattroff(win, A_REVERSE);
                }
        }

        box(win, 0, 0);
        wrefresh(win);
};
