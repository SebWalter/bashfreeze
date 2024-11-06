#include <memory>
#include <ncurses.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "class_Process.h"
#include "processreader.h"

#define COLUMCOUNT 2

using namespace std;


void draw_table(WINDOW *win, vector<unique_ptr<Process>> &processes, int selected_row) {
	//draw Headers
	mvwprintw(win, 1, 2, "PID");
	mvwprintw(win, 1, 12, "Name");

	mvwhline(win, 2, 1, ACS_HLINE, 30);

	//Draw rows
	
	for (int i = 0; i < processes.size(); i++) {
		if (i == selected_row) {
			wattron(win, A_REVERSE);
		}
		mvwprintw(win, 3+i, 2, "%d",  processes[i]->get_process_id());
		mvwprintw(win, 3+i, 12, "%s", processes[i]->get_name().c_str());
		wattroff(win, A_REVERSE);
	}
	box(win, 0,0);
	wrefresh(win);
}

int main() {
    int c;
    int selected_row = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();

    // Create a window for the table
    WINDOW *table_win = newwin(15, 35, 4, 4);
    box(table_win, 0, 0);
    mvwprintw(table_win, 0, 2, " Process Table ");
    vector<unique_ptr<Process>> processes = generate_process_vector();
    // Main loop
    while (1) {
        draw_table(table_win, processes, selected_row);

        c = wgetch(table_win);
        switch (c) {
            case KEY_UP:
                if (selected_row > 0) selected_row--;
                break;
            case KEY_DOWN:
                if (selected_row < 20 - 1) selected_row++;
                break;
            case 'q': // Quit program
                endwin();
                return 0;
        }
    }
}
