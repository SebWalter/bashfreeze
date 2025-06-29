#include "class_Process.h"
#include "processreader.h"
#include "processreader_worker.h"
#include "scrollable_table.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>
#include <vector>

struct tableSize {
        int rowsT;
        int rowsF;
        int columnsFT;
};
typedef struct tableSize tableSize;

static bool redraw = false;
static bool resize = false;

static int max_process_display_count = 30;

/* calculates the windows and table sizes for the two tables
 * @param freezedProzesses number of freezed processes to ensure the upper table has right size
 * @param rowsT pointer to main Table rows
 * @param rowsF pointer to freezedTable rows
 * @param columnsFT size of the columns of both tables
 * @return void
 */
static void handleTableDimensions(int freezedProzessesCount, tableSize *tableS) {
        int rows, columns;
        getmaxyx(stdscr, rows, columns);
        if (freezedProzessesCount <= 0) {
                tableS->rowsF = 0; // No freezed processes, table is hidden.
                tableS->rowsT = rows;
                tableS->columnsFT = columns; // Full width for the 'all' table.
                return;
        }
        tableS->rowsF = rows / 2;
        tableS->rowsT = max(0, rows - (tableS->rowsF + 1));
        tableS->columnsFT = columns; // Both tables use full width when both are visible.
}

// converts a tableSize struct so it can be used for the constructor.
// Because, rows have per eacht table OVERHEAD that we cannot fill with elements
static void handleTableSizes(tableSize *tableS) {
        tableS->rowsF = max(0, tableS->rowsF); // Ensure non-negative
        tableS->rowsT = max(0, tableS->rowsT);
        return;
}

static void update_maximal_process_display_count_per_table() {
	int rows, columns;
	getmaxyx(stdscr, rows, columns);
	// substract list overhead columns 
	rows -= 2 * (2 + 3);
	max_process_display_count = floor(rows/2);
	return;
}

class TableManager {
      private:
        Scrollable_Table freezed;
        Scrollable_Table all;
        // selected Table 0 is the upper (freezed)
        //		 1 is the lower (all)
        int selected_table;
        int freezed_count;
        int last_freezed_count = 0;
        int freezed_display_rows = 0;
        int all_display_rows = 0;
        static constexpr int overhead = 2 + 3; // WINDOW_FRAME_VERTICAL_PADDING + DATA_START_OFFSET_Y

        /* checks for tableMovement in specified table, and changes the selected to new val
         * @param table table where to check for movement
         * @param input is the current input character
         * @return void
         * */
        static void check_for_table_movement(Scrollable_Table *table, int input) {
                switch (input) {
                        case KEY_UP:
                        case 'k':
                                table->selected_up();
                                redraw = true;
                                break;
                        case KEY_DOWN:
                        case 'j':
                                table->selected_down();
                                redraw = true;
                                break;
                }
                return;
        }

      public:
        // constructor of table manager
        TableManager(tableSize *tS) : freezed(tS->rowsF, tS->columnsFT, 0, 0, tS->rowsF - 2), all(tS->rowsT, tS->columnsFT, 0, 0, tS->rowsT - 2) {
                this->selected_table = 1;
                this->freezed_count = 0;
        };

	void handleTableSizeChange() {
		int rows, columns;
		getmaxyx(stdscr, rows, columns);
		int available_rows = rows;
		int per_table_rows = 0;
		int freezee_table_size = this->freezed.get_process_count();
		if (freezee_table_size > 0) {
			per_table_rows = (available_rows - overhead * 2) / 2;
			if (per_table_rows < 1) per_table_rows = 1;
			this->freezed.resize_and_move(0, 0, 0, 0, per_table_rows);
			this->all.resize_and_move(0, 0, 0, 0, per_table_rows);
			this->freezed_display_rows = per_table_rows;
			this->all_display_rows = per_table_rows;
		} else {
			this->freezed.resize_and_move(0, 0, 0, 0, 0);
			this->all.resize_and_move(0, 0, 0, 0, available_rows - overhead);
			this->freezed_display_rows = 0;
			this->all_display_rows = available_rows - overhead;
		}
	}
        // draws both tables on the screen
        void printTables() {
                this->freezed.draw("freezed");
                this->all.draw("all");
        }
        void moveProcess() {
                if (this->selected_table == 0) {
                        unfreezeProcess(this->freezed.get_selected());
                } else
                        freezeProcess(this->all.get_selected());
                resize = true;
                return;
        }
        // handles the input
        void handleInput() {
                int c = getch();
                Scrollable_Table* current_table = (this->selected_table == 0) ? &this->freezed : &this->all;
                
                // Handle table movement first
                check_for_table_movement(current_table, c);
                
                // Then handle other commands
                switch (c) {
                        case 's':
                                redraw = true;
                                this->selected_table = (this->selected_table == 0) ? 1 : 0;
                                break;
                        case 'q':
                                destroy_worker();
                                endwin();
                                exit(EXIT_SUCCESS);
                                return;
                        case KEY_RESIZE:
                                resize = true;
                                return;
                        case KEY_ENTER:
                        case '\n':
                                moveProcess();
                                redraw = true;
                                break;
                }
        }
        void setFreezeProcesses(std::vector<std::unique_ptr<Process>> newProcessVector) {
                this->freezed.set_processes(std::move(newProcessVector));
        }

        void setAllProcesses(std::vector<std::unique_ptr<Process>> newProcessVector) {
                this->all.set_processes(std::move(newProcessVector));
        }
        void updateTableDimensions() {
                int columns;
                getmaxyx(stdscr, std::ignore, columns);
                int freezed_win_rows = this->freezed_display_rows + overhead;
                int all_win_rows = this->all_display_rows + overhead;
                int y_offset_all_table = (this->freezed_display_rows > 0) ? freezed_win_rows + 1 : 0;
                this->freezed.resize_and_move(freezed_win_rows, columns, 0, 0, this->freezed_display_rows);
                this->all.resize_and_move(all_win_rows, columns, 0, y_offset_all_table, this->all_display_rows);
        }
        void freezeProcess(int to_freeze_index) {
                int before = this->freezed.get_process_count();
                unique_ptr<Process> process_to_freeze = this->all.remove_process(to_freeze_index);
                if (process_to_freeze == NULL) {
                        cout << "process was null" << endl;
                        return;
                }
                this->freezed.add_process(std::move(process_to_freeze));
                this->freezed_count = this->freezed.get_process_count();
                if (before == 0 && this->freezed_count > 0) resize = true;
                return;
        }
        void unfreezeProcess(int to_unfreeze_index) {
                int before = this->freezed.get_process_count();
                unique_ptr<Process> process_to_unfreeze = this->freezed.remove_process(to_unfreeze_index);
                if (process_to_unfreeze == NULL) {
                        cout << "process was null" << endl;
                        return;
                }
                this->all.add_process(std::move(process_to_unfreeze));
                this->freezed_count = this->freezed.get_process_count();
                if (before > 0 && this->freezed_count == 0) resize = true;
                return;
        }
};
int main() {
        // initialize the screen
        initscr();
        noecho(); // no prnting of characters that user inputs
        cbreak(); // gives all input characters directly to program
        keypad(stdscr, true);
        start_color();
        // setting up timeout
        timeout(5000);

        if (init_worker() != 0) {
                endwin();
                cerr << "Failed to start proccessReader woker" << endl;
                exit(EXIT_FAILURE);
        }
        // init tables
        tableSize tableS;
        handleTableDimensions(0, &tableS);
        // WINDOW *freezed_window = newwin(tableS.rowsF, tableS.columnsFT, 0, 0); // Redundant
        // WINDOW *all_window = newwin(tableS.rowsT, tableS.columnsFT, tableS.rowsF + 1, 0); // Redundant

        // First time getting the Process vektor sequently
        // Else main would just wait
        vector<unique_ptr<Process>> process_vector = generate_process_vector();
        vector<unique_ptr<Process>> stoped_vector;
        int startT, startF, selectedF, selectedT = 0;

        // construct table Manager
        handleTableSizes(&tableS);
        TableManager manager = TableManager(&tableS);
        manager.setAllProcesses(std::move(process_vector)); // Pass by rvalue to enable move
        // this refresh somehow solves that, the screen is correctly drawn the first time
        refresh();
        manager.printTables();
        while (1) {
                manager.handleInput();
                if (resize == true || redraw == true) {
                        clear();
                        refresh();
                }
                if (resize == true) {
                        manager.updateTableDimensions();
                        manager.handleTableSizeChange();
                        resize = false;
                        redraw = true;
                }
                if (redraw == true) {
                        manager.printTables();
                        redraw = false;
                        refresh();
                }

                // Request new process information
                request_process_vector();
                try {
                        vector<unique_ptr<Process>> new_processes = get_process_vector();
                        cout << "Got " << new_processes.size() << " processes" << endl;
                        manager.setAllProcesses(std::move(new_processes));
                        redraw = true;
                } catch (const runtime_error& e) {
                        // No new process vector available yet, continue
                        continue;
                }
        }
}
