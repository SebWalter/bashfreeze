#include "scrollable_table.h"
#include <csignal>
#include <ncurses.h>
#include <stdexcept>
#include <stdlib.h> // For general utilities, often included with ncurses
#include <utility>
#include <vector>

#define BORDER_WIDTH 1 // Width of the border on each side (e.g., for box(win,0,0))
// static const int HEADER_HEIGHT = 1; // Height of the header text line itself. Replaced by
// COLUMN_HEADER_OFFSET_Y
#define WINDOW_FRAME_VERTICAL_PADDING 2 // Total lines used by top and bottom window borders
#define HEADER_AREA_HEIGHT 2            // Total lines used by title and column headers area (above data)

// New constants for drawing layout:
#define COLUMN_HEADER_OFFSET_Y 1  // Y-offset from window top for "PID", "Name" headers
#define SEPARATOR_LINE_OFFSET_Y 2 // Y-offset for the horizontal line below headers
#define DATA_START_OFFSET_Y 3     // Y-offset for the first line of data
#define INTER_COLUMN_SPACING 1    // Horizontal space between columns
#define NUMBER_OF_COLUMNS 2       // Number of columns (PID, Name)

Scrollable_Table::Scrollable_Table(int height, int width, int starty, int startx, int display_rows) {
        this->x = startx;
        this->y = starty;
        this->width = width;
        this->selected = 0;
        this->display_rows = display_rows;
        this->start_index = 0;
        this->end_index = display_rows - 1;
        if (height > 0 && width > 0) {
            this->win = newwin(height, width, starty, startx);
        }
}

void Scrollable_Table::resize_and_move(int height, int width, int starty, int startx, int display_rows) {
        this->y = starty;
        this->x = startx;
        this->width = width;
        this->display_rows = display_rows;

        if (this->win != nullptr) {
            delwin(this->win);
            this->win = nullptr;
        }

        if (height > 0 && width > 0) {
            this->win = newwin(height, width, starty, startx);
        }

        update_display_rows(display_rows);
}

int Scrollable_Table::get_selected() const {
        return this->selected;
}

std::unique_ptr<Process> Scrollable_Table::remove_process(int index) {
        if (index < 0 || index >= this->processes.size()) {
            return nullptr;
        }

        std::unique_ptr<Process> removed_process = std::move(this->processes[index]);
        this->processes.erase(this->processes.begin() + index);

        // Update selected and indices
        if (this->selected >= this->processes.size() && this->processes.size() > 0) {
            this->selected = this->processes.size() - 1;
        }

        return removed_process;
}

void Scrollable_Table::set_processes(std::vector<std::unique_ptr<Process>> new_processes) {
        this->processes = std::move(new_processes);

        if (this->selected >= this->processes.size() && this->processes.size() > 0) {
            this->selected = this->processes.size() - 1;
        }
}

int Scrollable_Table::get_process_count() const {
        return this->processes.size();
}

int Scrollable_Table::get_display_rows() const {
        return this->display_rows;
}

void Scrollable_Table::selected_down() {
        // If new selected is out of the frame --> check if i reached the lower end of the table.
        // 			No -> move frame
        // 			Yes -> ignore you reached end of table can't go down further
        if (this->processes.size() <= this->selected) {
                return;
        }
        if (this->end_index < (this->selected + 1)) {
                this->start_index++;
                this->end_index++;
                this->selected++;
                return;
        }
        this->selected++;
        return;
}

void Scrollable_Table::selected_up() {
        // If new selected is out of he frame --> check if i reached th upper end of the table
        // 			No -> Move frame
        // 			Yes --> do nothing
        if (this->start_index < 0) {
                return;
        }
        if (this->start_index > (this->selected - 1) && this->start_index > 0) {
                this->start_index--;
                this->end_index--;
                this->selected--;
                return;
        }
        this->selected--;
        return;
}

void Scrollable_Table::update_display_rows(int new_display_rows) {
	this->display_rows = new_display_rows;
	this->end_index = this->start_index + new_display_rows - 1;
	// check if selected is out of the new frame
	if (this->selected < this->start_index) {
		this->selected = this->start_index;
	} else if (this->selected > this->end_index && this->end_index >= this->start_index) {
		this->selected = this->end_index;
	}
}
std::unique_ptr<Process> Scrollable_Table::remove_process_selected() {
	// check if current selected is broken
	if (this->selected < this->start_index || this->selected > this->end_index) {
		throw std::out_of_range("Table selected is out of range");
	}
	int index_to_remove = this->selected;
	// remove the process and return it
	std::unique_ptr<Process> removed_process = std::move(this->processes[this->selected]);
	this->processes.erase(this->processes.begin() + this->selected);
	this->selected_up();
	return  removed_process;
}
void Scrollable_Table::add_process(std::unique_ptr<Process> process_to_add) {
	this->processes.push_back(std::move(process_to_add));
	// maby here sort
}

void Scrollable_Table::draw(const char* title) {
    if (this->win == nullptr || this->display_rows <= 0) {
        return;
    }

    wclear(this->win);
    box(this->win, 0, 0);

    // Draw title
    if (title != nullptr) {
        mvwprintw(this->win, 0, 2, " %s ", title);
    }

    // Draw header
    mvwprintw(this->win, COLUMN_HEADER_OFFSET_Y, 2, "PID");
    mvwprintw(this->win, COLUMN_HEADER_OFFSET_Y, 10, "Name");

    // Draw separator
    mvwhline(this->win, SEPARATOR_LINE_OFFSET_Y, 1, ACS_HLINE, this->width - 2);

    // Draw processes
    int row = DATA_START_OFFSET_Y;
    for (int i = this->start_index; i <= this->end_index && i < this->processes.size(); i++) {
        if (i == this->selected) {
            wattron(this->win, A_REVERSE);
        }

        mvwprintw(this->win, row, 2, "%d", this->processes[i]->get_process_id());
        mvwprintw(this->win, row, 10, "%s", this->processes[i]->get_name().c_str());

        if (i == this->selected) {
            wattroff(this->win, A_REVERSE);
        }

        row++;
    }

    wrefresh(this->win);
}


