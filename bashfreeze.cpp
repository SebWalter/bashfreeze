#include "class_Process.h"
#include "processreader.h"
#include "processreader_worker.h"
#include "scrollable_table.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <tuple>
#include <vector>

// overhead eacht table has
#define OVERHEAD 4

struct tableSize {
        int rowsT;
        int rowsF;
        int columnsFT;
};
typedef struct tableSize tableSize;

/* calculates the windows and table sizes for the two tables
 * @param freezedProzesses number of freezed processes to ensure the upper table has right size
 * @param rowsT pointer to main Table rows
 * @param rowsF pointer to freezedTable rows
 * @param columnsFT size of the columns of both tables
 * @return void
 */
static void handleTableDimensions(int freezedProzessesCount, tableSize *tableS) {
        int rows, columns;
        int tableOverhead = OVERHEAD; // everyTablehas currently 4 lines overhead
        getmaxyx(stdscr, rows, columns);
        // maby change later currently if freezedProzzeses == 0 than do not display second table
        if (freezedProzessesCount <= 0) {
                tableS->rowsF = 0;
                tableS->columnsFT = columns;
                tableS->rowsT = (rows - tableOverhead);
                return;
        }
        tableS->rowsF = freezedProzessesCount + OVERHEAD;
        tableS->rowsT = rows - (tableS->rowsF + 1); // one line as space between tables
        tableS->columnsFT = columns;
        return;
}
// converts a tableSize struct so it can be used for the constructor.
// Because, rows have per eacht table OVERHEAD that we cannot fill with elements
static void handleTableSizes(tableSize *tableS) {
        tableS->rowsF = max(0, tableS->rowsF - OVERHEAD);
        tableS->rowsT = max(0, tableS->rowsT - OVERHEAD);
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
                                table->selected_decrement();
                                break;
                        case KEY_DOWN:
                        case 'j':
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
        // handles the input
        void handleInput() {
		int c = -1;
                if (this->selected_table == 0) {
			c = this->freezed.get_input();
                        this->check_for_table_movement(&this->freezed, c);
                        // todo: add check for enter for unfreeze process
                } else {
			c = this->all.get_input();
                        this->check_for_table_movement(&this->all, c);
                        // todo: add check for enter for freeze process
                }
                switch (c) {
                        case 's':
                                if (this->selected_table == 0) {
                                        this->selected_table = 1;
                                        break;
                                }
                                this->selected_table = 0;
                                break;

                        case 'q':
				destroy_worker();
				exit(EXIT_SUCCESS);
                                // do a exit
                                return;
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

                return;
        }
};
int main() {
        // initialize the screen
        initscr();
        noecho(); // no prnting of characters that user inputs
        cbreak(); // gives all input characters directly to program
        start_color();

        if (init_worker() != 0) {
                endwin();
                cerr << "Failed to start proccessReader woker" << endl;
                exit(EXIT_FAILURE);
        }
        request_process_vector();
        // init tables
        tableSize tableS;
        handleTableDimensions(0, &tableS);
        WINDOW *freezed_window = newwin(tableS.rowsF, tableS.columnsFT, 0, 0);
        WINDOW *all_window = newwin(tableS.rowsT, tableS.columnsFT, tableS.rowsF + 1, 0);
        // set keypads to true so input can be seen
        keypad(freezed_window, TRUE);
        keypad(all_window, TRUE);

        // First time getting the Process vektor sequently
        // Else main would just wait
        vector<unique_ptr<Process>> process_vector;
        while (1) {
                try {
                        process_vector = get_process_vector();
                        break;
                } catch (...) {
                };
        }
        vector<unique_ptr<Process>> stoped_vector;
        int startT, startF, selectedF, selectedT = 0;

        // construct table Manager
        handleTableSizes(&tableS);
        TableManager manager = TableManager(&tableS);
        manager.setAllProcesses(&process_vector);
        while (1) {
                manager.updateTableDimensions();
                manager.handleInput();
                manager.printTables();
        }
}
