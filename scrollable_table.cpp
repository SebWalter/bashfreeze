#include <memory>
#include <ncurses.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "class_Process.h"
#include "processreader.h"

#define COLUMCOUNT 2

using namespace std;

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
}

int main() {
    int c;
    int selected_row = 0;

    initscr();
    cbreak();
    noecho();
    start_color();

    // Create a window for the table
    WINDOW *table_win = newwin(15, 70, 4, 4);
    box(table_win, 0, 0);
    mvwprintw(table_win, 0, 2, " Process Table ");
    keypad(table_win, TRUE); // Enable keypad for the table window

    vector<unique_ptr<Process>> processes = generate_process_vector();

    int start = 0;
    int end = 10; // Show 10 rows at a time (adjust based on your window height)

    // Main loop
    while (1) {
        draw_table(table_win, processes, selected_row, start, end);
        
        // Adjust scrolling
        if (selected_row >= end-1 && selected_row < processes.size()) {
            start++;
            end++;
        }
        if (selected_row < start +1 && start > 0) {
            start--;
            end--;
        }

        c = wgetch(table_win); // Use wgetch on the table window
        switch (c) {
            case KEY_UP:
                if (selected_row > 0) { selected_row--; }
                break;
            case KEY_DOWN:
                if (selected_row < processes.size() - 1) { selected_row++; }
                break;
            case 'q': // Quit program
                endwin();
                return 0;
        }
    }
}

