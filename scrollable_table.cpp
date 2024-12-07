#include <memory>
#include <ncurses.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>
#include "class_Process.h"
#include "processreader.h"

#define COLUMCOUNT 2

using namespace std;

class Scrollable_Table{
private:
	WINDOW *win;
	vector<unique_ptr<Process>> processes;
	int rows;
	int columns;
	int start;
	int end;
	int selected;
	int position_x;
	int position_y;

public:
	Scrollable_Table(int rows, int columns, int position_x, int position_y) {
		this->win = newwin(rows, columns, position_x, position_y);
		box(this->win, 0, 0);
		mvwprintw(this->win, 0, 2, " Process Table ");
    		keypad(this->win, TRUE); // Enable keypad for the table window


		this->rows = rows;
		this->columns = columns;
		this->position_x = position_x;
		this->position_y = position_y;
		this->start = 0;			
		this->end = rows - 4;			//last 4 lines are for window styling
	}
	Scrollable_Table(vector<unique_ptr<Process>> processes, int rows, int columns, int position_x, int position_y) {
		this->processes = std::move(processes);
		this->win = newwin(rows, columns, position_x, position_y);
		this->win = newwin(rows, columns, position_x, position_y);
		box(this->win, 0, 0);
		mvwprintw(this->win, 0, 2, " Process Table ");
		this->rows = rows;
		this->columns = columns;
		this->position_x = position_x;
		this->position_y = position_y;
		this->start = 0;
		this->end = rows - 4;			//last 4 lines are for window styling
	}
	vector<unique_ptr<Process>> *get_processes() {
		if (0  > processes.size()) {
			return &processes;
		}
		return NULL;
	}



	//update methods
	void set_processes(vector<unique_ptr<Process>> processes) {
		this->processes = std::move(processes);
	}

	void set_window_dimensions(int rows,int columns) {
		this->rows = rows;
		this->columns = columns;
	}

	void set_window_position(int x, int y) {
		this->position_x = x;
		this->position_y = y;
	}

	void set_selected(int selected) {
		this->selected = selected;
	}

	void selected_increment() {
		if (this->selected < this->processes.size() - 1) { 
			this->selected++; 
		}
	}

	void selected_decement() {
		if (this->selected > 0) {
			this->selected--; 
		}
	}

	int get_input() {
		return wgetch(this->win);
	}

	void draw_table() {
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
	}	


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
	
	//adjust scrolling
	 if (selected_row >= end-1 && selected_row < processes.size()) {
            start++;
            end++;
        }
        if (selected_row < start +1 && start > 0) {
            start--;
            end--;
        }

	
}

int main() {
    int c;
    int selected_row = 0;

    initscr();
    cbreak();
    noecho();
 start_color();

    // Create a window for the table
    int window_Height = 45; 			//todo figure out how big a window can be
    int window_Width = 70;
    Scrollable_Table table = Scrollable_Table(window_Width, window_Height, 4,4);
    
    vector<unique_ptr<Process>> processes = generate_process_vector();

    table.set_processes(std::move(processes));


    // Main loop
    while (1) {
       	table.draw_table(); 
       
        c = table.get_input(); // Use wgetch on the table window
        switch (c) {
        	case KEY_UP :
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
}

