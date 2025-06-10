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
                                             // The previous 'columns - 3' is removed for simplicity.
                return;
        }
        tableS->rowsF = Scrollable_Table::get_required_total_height(freezedProzessesCount);
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

        /* checks for tableMovement in specified table, and changes the selected to new val
         * @param table table where to check for movement
         * @param input is the current input character
         * @return void
         * */
        static void check_for_table_movement(Scrollable_Table *table, int input) {
                switch (input) {
                        case KEY_UP:
                        case 'k':
                                redraw = true;
                                table->selected_decrement();
                                break;
                        case KEY_DOWN:
                        case 'j':
                                redraw = true;
                                table->selected_increment();
                }
                return;
        }

      public:
        // constructor of table manager
        TableManager(tableSize *tS) : freezed(tS->rowsF, tS->columnsFT, 0, 0), all(tS->rowsT, tS->columnsFT, 0, 0) {
                all.print_window_name("all");
                this->selected_table = 1;
                this->freezed_count = 0;
        };

	void handleTableSizeChange() {
		// calculate the default table size 
		// use here a global static
		int all_table_size = this->all.get_process_count();
		int freezee_table_size = this->freezed.get_process_count();
		// calculate what each table size should be
		if (all_table_size >= max_process_display_count && freezee_table_size >= max_process_display_count) {
			// set both to max_process_display count
		}
		else if (all_table_size >= max_process_display_count) {
			int max_size = max_process_display_count *2;
			max_size -= freezee_table_size;
			// freezedTable set to frezzed table size
			this->freezed.set_display_rows(freezee_table_size);
			// all -> max_size
			this->all.set_display_rows(max_size);
		}
		else if (freezee_table_size >= max_process_display_count) {
			int max_size = max_process_display_count *2;
			max_size -= all_table_size;
			// all --> all table size
			this->all.set_display_rows(all_table_size);
			// freezed --> max size
			this->freezed.set_display_rows(max_size);
		}
		else {
			// all -> all table size
			this->all.set_display_rows(all_table_size);
			// freezed --> freezed table size
			this->freezed.set_display_rows(freezee_table_size);
		}
		// finish yeah
		return;



	}
        // draws both tables on the screen
        void printTables() {
                this->freezed.draw_table();
                this->all.draw_table();
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
                if (this->selected_table == 0) {
                        check_for_table_movement(&this->freezed, c);
                } else {
                        check_for_table_movement(&this->all, c);
                }
                switch (c) {
                        case 's':
                                redraw = true;
                                if (this->selected_table == 0) {
                                        this->selected_table = 1;
                                        cout << "Table switch to 1, current table:" << this->selected_table << endl;
                                        break;
                                }
                                this->selected_table = 0;
                                cout << "Table switch to , current table:" << this->selected_table << endl;
                                break;

                        case 'q': // todo: breaks the terminal lul
                                destroy_worker();
                                endwin();
                                exit(EXIT_SUCCESS);
                                // do a exit
                                return;
                        case KEY_RESIZE:
                                resize = true;
                                return;
                        case KEY_ENTER:
                        case '\n':
                                moveProcess();
                                redraw = true;
                }
        }
        void setFreezeProcesses(std::vector<std::unique_ptr<Process>> newProcessVector) {
                this->freezed.set_processes(std::move(newProcessVector));
        }

        void setAllProcesses(std::vector<std::unique_ptr<Process>> newProcessVector) {
                this->all.set_processes(std::move(newProcessVector));
        }
        void updateTableDimensions() {
                tableSize tableS;
                handleTableDimensions(this->freezed_count, &tableS);

                // Update 'freezed' table
                this->freezed.resize_and_relocate_window(tableS.rowsF, tableS.columnsFT, 0, 0);
                if (tableS.rowsF > 0) { // Only print name if the window has some height
                        this->freezed.print_window_name("freezed");
                }

                // Calculate y-offset for 'all' table
                int y_offset_all_table = (tableS.rowsF > 0) ? tableS.rowsF + 1 : 0;

                // Update 'all' table
                this->all.resize_and_relocate_window(tableS.rowsT, tableS.columnsFT, 0, y_offset_all_table);
                if (tableS.rowsT > 0) { // Only print name if the window has some height
                        this->all.print_window_name("all");
                }

                return;
        }
        void freezeProcess(int to_freeze_index) {
                unique_ptr<Process> process_to_freeze = this->all.remove_process(to_freeze_index);
                if (process_to_freeze == NULL) {
                        cout << "process was null" << endl;
                        return;
                }
                this->freezed.add_process(std::move(process_to_freeze));
                this->freezed_count++;
                return;
        }
        void unfreezeProcess(int to_unfreeze_index) {
                unique_ptr<Process> process_to_unfreeze = this->freezed.remove_process(to_unfreeze_index);
                if (process_to_unfreeze == NULL) {
                        cout << "process was null" << endl;
                        return;
                }
                this->all.add_process(std::move(process_to_unfreeze));
                this->freezed_count--;
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
                        resize = false;
                        redraw = true;
                }
                if (redraw == true) {
			manager.handleTableSizeChange();
                        manager.printTables();
                        redraw = false;
                        refresh();
                }
        }
}
