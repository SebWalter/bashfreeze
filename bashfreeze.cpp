#include "class_Process.h"
#include "processreader.h"
#include "processreader_worker.h"
#include "scrollable_table.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>
#include <vector>

// overhead eacht table has
#define OVERHEAD 4

struct tableSize {
        int rowsT;
        int rowsF;
        int columnsFT;
};
typedef struct tableSize tableSize;

static bool redraw = false;
static bool resize = false;
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
                tableS->rowsF = 0;
                tableS->rowsT = max(1, rows - OVERHEAD); // Ensure at least 1 row
                tableS->columnsFT = columns - 3;
                return;
        }
        tableS->rowsF = freezedProzessesCount + OVERHEAD;
        tableS->rowsT = max(1, rows - (tableS->rowsF + 1)); // Ensure at least 1 row
        tableS->columnsFT = columns;
}
// converts a tableSize struct so it can be used for the constructor.
// Because, rows have per eacht table OVERHEAD that we cannot fill with elements
static void handleTableSizes(tableSize *tableS) {
        tableS->rowsF = max(0, tableS->rowsF - OVERHEAD);
        tableS->rowsT = max(0, tableS->rowsT - OVERHEAD);
        // Ensure at least 1 row if possible
        tableS->rowsT = max(1, tableS->rowsT);
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

        // draws both tables on the screen
        void printTables() {
                this->freezed.draw_table();
                this->all.draw_table();
        }
	void moveProcess() {
		if (this->selected_table == 0) {
			unfreezeProcess(this->freezed.get_selected());
		}
		else freezeProcess(this->all.get_selected());
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
        void setFreezeProcesses(vector<unique_ptr<Process>> *newProcessVector) {
                this->freezed.set_processes(newProcessVector);
                return;
        }

        void setAllProcesses(vector<unique_ptr<Process>> *newProcessVector) {
                this->all.set_processes(newProcessVector);
                return;
        }
        void updateTableDimensions() {
                // first update Dimensions
                tableSize tableS;
                handleTableDimensions(this->freezed_count, &tableS);
                if (this->freezed_count > 0) {
                        this->freezed.update_window_dimensions(tableS.rowsF, tableS.columnsFT, 0, 0);
                        freezed.print_window_name("freezed");
                }
                this->all.update_window_dimensions(tableS.rowsT, tableS.columnsFT, 0, tableS.rowsF + 2);
                this->all.print_window_name("all");
                // second update Table
                handleTableSizes(&tableS);
                handleTableSizes(&tableS);
                this->freezed.set_table_dimensions(tableS.rowsF, tableS.columnsFT);
                this->all.set_table_dimensions(tableS.rowsT, tableS.columnsFT);
                cout << "all: rows" << tableS.rowsT << "  columns:" << tableS.columnsFT << endl;
                cout << "freezed: rows" << tableS.rowsF << "  columns:" << tableS.columnsFT << endl;
                return;
        }
	void freezeProcess(int to_freeze_index) {
		unique_ptr<Process> process_to_freeze = this->all.remove_process(to_freeze_index);
		this->freezed.add_process(std::move(process_to_freeze));
		return;
	}
	void unfreezeProcess(int to_unfreeze_index) {
		unique_ptr<Process> process_to_unfreeze = this->freezed.remove_process(to_unfreeze_index);
		this->all.add_process(std::move(process_to_unfreeze));
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
        WINDOW *freezed_window = newwin(tableS.rowsF, tableS.columnsFT, 0, 0);
        WINDOW *all_window = newwin(tableS.rowsT, tableS.columnsFT, tableS.rowsF + 1, 0);

        // First time getting the Process vektor sequently
        // Else main would just wait
        vector<unique_ptr<Process>> process_vector = generate_process_vector();
        vector<unique_ptr<Process>> stoped_vector;
        int startT, startF, selectedF, selectedT = 0;

        // construct table Manager
        handleTableSizes(&tableS);
        TableManager manager = TableManager(&tableS);
        manager.setAllProcesses(&process_vector);
	// this refresh somehow solves that, the screen is correctly drawn the first time
	refresh();
        manager.printTables();
        while (1) {
		manager.handleInput();
                if (resize == true) {
                        manager.updateTableDimensions();
                        resize = false;
                        redraw = true;
                }
                if (redraw == true) {
                        manager.printTables();
                        redraw = false;
			refresh();
                }
        }
}
