#include "scrollable_table.h"
#include "class_Process.h"
#include "processreader.h"
#include "processreader_worker.h"
#include <cstdlib>
#include <iostream>
#include <ncurses.h>

/* calculates the windows and table sizes for the two tables
 * @param freezedProzesses number of freezed processes to ensure the upper table has right size
 * @param rowsT pointer to main Table rows 
 * @param rowsF pointer to freezedTable rows
 * @param columnsFT size of the columns of both tables
 * @return void
 */
static void handleTableSizes(int freezedProzessesCount, int *rowsT, int *rowsF, int *columnsFT ) {
	int rows, columns;
	int tableOverhead = 4;		//everyTablehas currently 4 lines overhead
	getmaxyx(stdscr, rows, columns);
	//maby change later currently if freezedProzzeses == 0 than do not display second table	
	if (freezedProzessesCount <= 0) {
		*rowsF = 0;
		*columnsFT = columns;
		*rowsT = (rows - tableOverhead);
		return;
	}
	*rowsF = freezedProzessesCount + 4;
	*rowsT = rows - (*rowsF + 1);		//one line as space between tables
	*columnsFT = columns;
	return;
}

int main() {
	//initialize the screen
	initscr();
	noecho();		//no prnting of characters that user inputs
	cbreak();		//gives all input characters directly to program
	
	if (init_worker() != 0) {
		endwin();
		cerr<<"Failed to start proccessReader woker"<<endl;
		exit(EXIT_FAILURE);
	}
		
	






}
