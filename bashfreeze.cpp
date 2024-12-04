#include "scrollable_table.h"
#include "class_Process.h"
#include "processreader.h"
#include "processreader_worker.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <vector>

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
static void handleTableSizes(int freezedProzessesCount, tableSize *tableS) {
	int rows, columns;
	int tableOverhead = 4;		//everyTablehas currently 4 lines overhead
	getmaxyx(stdscr, rows, columns);
	//maby change later currently if freezedProzzeses == 0 than do not display second table	
	if (freezedProzessesCount <= 0) {
		tableS->rowsF = 0;
		tableS->columnsFT = columns;
		tableS->rowsT = (rows - tableOverhead);
		return;
	}
	tableS->rowsF = freezedProzessesCount + 4;
	tableS->rowsT = rows - (tableS->rowsF + 1);		//one line as space between tables
	tableS->columnsFT = columns;
	return;
}

	


int main() {
	//initialize the screen
	initscr();
	noecho();		//no prnting of characters that user inputs
	cbreak();		//gives all input characters directly to program
	start_color();	

	if (init_worker() != 0) {
		endwin();
		cerr<<"Failed to start proccessReader woker"<<endl;
		exit(EXIT_FAILURE);
	}
	//init tables
	tableSize tableS;
	handleTableSizes(0, &tableS);
	WINDOW *freezed_window = newwin(tableS.rowsF, tableS.columnsFT, 0, 0);
	WINDOW *all_window = newwin(tableS.rowsT, tableS.columnsFT, tableS.rowsF + 1, 0);
	//set keypads to true so input can be seen
	keypad(freezed_window, TRUE);
	keypad(all_window, TRUE);



	//First time getting the Process vektor sequently
	//Else main would just wait
	vector<unique_ptr<Process>> process_vector = get_process_vector();
	vector<unique_ptr<Process>> stoped_vector;
	int startT, startF, selectedF, selectedT = 0;	
	int endF = tableS.rowsF - 4;			//-4 because last 4 lines are can't be displayed
	int endT = tableS.rowsT - 4;

	while(1) {
				



	}
	






}
