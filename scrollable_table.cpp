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

Scrollable_Table::Scrollable_Table(int y, int display_rows) {
        this->x = 0; // always whole width so start with left
        this->y = 0;
        this->selected = 0;
        this->display_rows = display_rows;
        // setup first start index
        this->start_index = 0;
        this->end_index = display_rows - 1; // process table is 0 indexed -> -1
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
	// calculate how much the diffenrence is
	int current_display_rows = this->display_rows;
	if (current_display_rows > new_display_rows) {
		int offset = current_display_rows - new_display_rows;
		this->end_index -= offset;
	} else {
		this->end_index = this->start_index;
	}
	// check if selected is out of the new frame
	if (this->selected < this->start_index) {
		this->selected = this->start_index;
		return;
	}
	if (this->selected > this->end_index) {
		this->selected = this->end_index;
		return;
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

void Scrollable_Table::move_table(int new_y) {
	this->y = new_y;
	return;
}
void Scrollable_Table::when_rezise() {
	//todo: here stuff like width get's updated

}
void Scrollable_Table::draw() {
//todo: here do the drawing stuff};


