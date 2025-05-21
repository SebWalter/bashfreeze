#include "scrollable_table.h"
#include "class_Process.h"
#include "processreader.h"
#include <algorithm> // For std::max and std::min
#include <memory>
#include <ncurses.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#define COLUMCOUNT 2

using namespace std;

/* creates a new window, with a box, and keypad enabled
 * @param rows: how many rows the windows should have
 * @param column: how many columns the window should have
 * @param x: the position of the x-coordinate
 * @param y: the position of the y-coordinate
 */
WINDOW *init_new_window(int rows, int columns, int x, int y) {
        WINDOW *win = newwin(rows, columns, x, y);
        box(win, 0, 0);
        keypad(win, true);
        // setting waiting delay for input
        return win;
}

Scrollable_Table::Scrollable_Table(int displayable_rows, int columns, int position_x, int position_y)
    : columns(columns), selected(0) {
    this->rows = std::max(0, displayable_rows);
    int window_actual_height = this->rows + 4;
    this->win = init_new_window(window_actual_height, this->columns, position_y, position_x); // Corrected order of position_x and position_y
    this->start = 0;
    this->end = this->rows;
}

Scrollable_Table::Scrollable_Table(vector<unique_ptr<Process>> processes, int displayable_rows, int columns, int position_x,
                                   int position_y)
    : processes(std::move(processes)), columns(columns), selected(0) {
    this->rows = std::max(0, displayable_rows);
    int window_actual_height = this->rows + 4;
    this->win = init_new_window(window_actual_height, this->columns, position_y, position_x); // Corrected order of position_x and position_y
    this->start = 0;
    this->end = this->rows;
}
std::vector<std::unique_ptr<Process>>* Scrollable_Table::get_processes() {
    return &this->processes;
}
void Scrollable_Table::print_window_name(const char *name) {
        if (this->win == NULL) {
                return;
        }
        mvwprintw(this->win, 0, 2, " %s ", name);
        return;
}

// update methods
void Scrollable_Table::set_processes(vector<unique_ptr<Process>> *processes) {
        this->processes = std::move(*processes);
}
std::unique_ptr<Process> Scrollable_Table::remove_process(int index_to_remove) {
    int vecSize = this->processes.size();
    // Add check for index_to_remove < 0
    if (index_to_remove < 0 || index_to_remove >= vecSize || vecSize <= 0) {
        return nullptr; // Return nullptr if index is invalid or vector is empty
    }

    // It's generally safer to retrieve the element to be returned before modifying the vector structure
    // if the element itself is being moved out.
    std::unique_ptr<Process> removed_process = std::move(this->processes.at(index_to_remove));
    this->processes.erase(this->processes.begin() + index_to_remove);

    // Adjust selected, start, and end indices
    int new_size = this->processes.size();

    if (new_size == 0) {
        this->selected = 0;
        this->start = 0;
        this->end = 0;
    } else {
        // Adjust 'selected' index
        if (this->selected >= new_size) {
            this->selected = new_size - 1; // Select the new last element
        }
        // this->selected cannot be < 0 here if index_to_remove was valid and selection was valid before removal.

        // Adjust 'start' to ensure 'selected' is visible and 'start' is valid
        if (this->selected < this->start) { // If selected item is now above viewport
            this->start = this->selected;
        } else if (this->selected >= this->start + this->rows) { // If selected item is below viewport (rows is capacity)
            this->start = this->selected - this->rows + 1;
        }
        // Cap 'start': must be >= 0 and allow for a full viewport if possible, but not beyond new_size - rows.
        // Make sure start is not negative first.
        this->start = std::max(0, this->start); 
        // Then ensure start does not exceed the maximum possible start index.
        this->start = std::min(this->start, std::max(0, new_size - this->rows));


        // Recalculate 'end' based on the new 'start' and 'rows' (capacity)
        this->end = this->start + this->rows;
        // Cap 'end' at the new_size of the process list
        this->end = std::min(this->end, new_size);
    }
    return removed_process;
}
void Scrollable_Table::add_process(std::unique_ptr<Process> process_to_add) {
	this->processes.push_back(std::move(process_to_add));
	return;
}

void Scrollable_Table::set_table_dimensions(int rows, int columns) {
        this->rows = rows;
        this->columns = columns;
}

void Scrollable_Table::update_window_dimensions(int x, int y) {
        if (this->win != NULL) {

                delwin(this->win);
        }
        this->win = init_new_window(this->rows, this->columns, y, x); // Corrected order of x and y
        return;
}

void Scrollable_Table::update_window_dimensions(int new_displayable_rows, int new_columns, int new_y_coord, int new_x_coord) {
    this->rows = std::max(0, new_displayable_rows);
    this->columns = new_columns;
    int window_actual_height = this->rows + 4;
    if (this->win != NULL) {
        delwin(this->win);
    }
    this->win = init_new_window(window_actual_height, this->columns, new_y_coord, new_x_coord);

    if (this->start >= (int)this->processes.size() && !this->processes.empty()) {
        this->start = std::max(0, (int)this->processes.size() - 1);
    }
    if (this->processes.empty()) {
        this->start = 0;
    }
    this->end = std::min(this->start + this->rows, (int)this->processes.size());
    if (this->selected < this->start || this->selected >= this->end) {
        this->selected = this->start;
        if (this->processes.empty() || this->rows == 0) {
            this->selected = 0;
            this->start = 0;
            this->end = 0;
        }
    }
}

void Scrollable_Table::set_selected(int selected) {
        this->selected = selected;
}

int Scrollable_Table::get_selected() {
	return this->selected;
}

void Scrollable_Table::selected_increment() {
        if (this->selected < this->processes.size() - 1) {
                this->selected++;
                if (this->end < this->selected && this->end < processes.size()) {
                        this->end++;
                        this->start++;
                }
        }
}

void Scrollable_Table::selected_decrement() {
        if (this->selected > 0) {
                this->selected--;
                if (this->selected < this->start && this->start > 0) {
                        this->start--;
                        this->end--;
                }
        }
}

void Scrollable_Table::draw_table() {
        if (this->win == NULL) {
                return;
        }
        int column_width = (this->columns - 2) / 2;

        // Draw Headers
        mvwprintw(win, 1, 1, "PID");
        mvwprintw(win, 1, column_width + 1, "Name");

        mvwhline(win, 2, 1, ACS_HLINE, this->columns - 2);

        // Draw rows
        for (int i = start; i < end && i < processes.size(); i++) {
                if (i == this->selected) {
                        wattron(win, A_REVERSE);
                }
                mvwprintw(win, 3 + (i - start), 2, "%d", processes[i]->get_process_id());
                mvwprintw(win, 3 + (i - start), column_width + 1, "%s", processes[i]->get_name().c_str());
                wattroff(win, A_REVERSE);
        }
        box(win, 0, 0);
        wrefresh(win);
};

void draw_table(WINDOW *win, vector<unique_ptr<Process>> &processes, int selected_row, int start, int end) {
        // Get window dimensions
        int width, height;
        getmaxyx(win, height, width);

        // We have 2 columns when more adjust here
        int column_width = (width - 2) / 2;

        // Draw Headers
        mvwprintw(win, 1, 1, "PID");
        mvwprintw(win, 1, column_width + 1, "Name");

        mvwhline(win, 2, 1, ACS_HLINE, width - 2);

        // Draw rows
        for (int i = start; i < end && i < processes.size(); i++) {
                if (i == selected_row) {
                        wattron(win, A_REVERSE);
                }
                mvwprintw(win, 3 + (i - start), 2, "%d", processes[i]->get_process_id());
                mvwprintw(win, 3 + (i - start), column_width + 1, "%s", processes[i]->get_name().c_str());
                wattroff(win, A_REVERSE);
        }
        box(win, 0, 0);
        wrefresh(win);

        // adjust scrolling
        if (selected_row >= end - 1 && selected_row < processes.size()) {
                start++;
                end++;
        }
        if (selected_row < start + 1 && start > 0) {
                start--;
                end--;
        }
}
/*
int main() {
        int c;
        int selected_row = 0;

        initscr();
        cbreak();
        noecho();
        start_color();

        // Create a window for the table
        int window_Height = 45; // todo figure out how big a window can be
        int window_Width = 30;
        Scrollable_Table table = Scrollable_Table(window_Height, window_Width, 4, 4);
        table.print_window_name("process_table");

        vector<unique_ptr<Process>> processes = generate_process_vector();

        table.set_processes(std::move(processes));

        // Main loop
        while (1) {
                table.draw_table();

                c = table.get_input(); // Use wgetch on the table window
                switch (c) {
                        case KEY_UP:
                        case 'k':
                                table.selected_decement();
                                break;
                        case KEY_DOWN:
                        case 'j':
                                table.selected_increment();
                                break;
                        case 'q': // Quit program
                                endwin();
                                return 0;
                }
        }
} */
