#include "scrollable_table.h"
#include "class_Process.h"
#include "processreader.h"
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
        return win;
}

Scrollable_Table::Scrollable_Table(int rows, int columns, int position_x, int position_y) {
        if (rows > 0) {
                this->win = init_new_window(rows, columns, position_x, position_y);
        } else {
                this->win = NULL;
        }
        this->selected = 0;

        this->rows = rows;
        this->columns = columns;
        this->start = 0;
        this->end = rows - 4; // last 4 lines are for window styling
}

Scrollable_Table::Scrollable_Table(vector<unique_ptr<Process>> processes, int rows, int columns, int position_x,
                                   int position_y) {
        this->processes = std::move(processes);
        if (rows > 0) {
                this->win = init_new_window(rows, columns, position_x, position_y);
        } else {
                this->win = NULL;
        }
        this->rows = rows;
        this->columns = columns;
        this->start = 0;
        this->end = rows - 4; // last 4 lines are for window styling
        this->selected = 0;
}
vector<unique_ptr<Process>> *Scrollable_Table::get_processes() {
        if (0 > processes.size()) {
                return &processes;
        }
        return NULL;
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

void Scrollable_Table::set_table_dimensions(int rows, int columns) {
        this->rows = rows;
        this->columns = columns;
}

void Scrollable_Table::update_window_dimensions(int x, int y) {
        if (this->win != NULL) {

                delwin(this->win);
        }
        this->win = init_new_window(this->rows, this->columns, x, y);
        return;
}

void Scrollable_Table::update_window_dimensions(int rows, int columns, int x, int y) {
        if (this->win != NULL) {
                delwin(this->win);
        }
        this->win = init_new_window(rows, columns, x, y);
        return;
}

void Scrollable_Table::set_selected(int selected) {
        this->selected = selected;
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

int Scrollable_Table::get_input() {
        return wgetch(this->win);
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
