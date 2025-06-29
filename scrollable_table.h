#ifndef SCROLLABLE_TABLE_H
#define SCROLLABLE_TABLE_H

#include "class_Process.h"
#include <memory>
#include <ncurses.h>
#include <vector>

class Scrollable_Table {
private:
    WINDOW *win = nullptr;
    std::vector<std::unique_ptr<Process>> processes;
    int x;
    int y;
    int display_rows; // How many processes can be displayed (set by manager)
    int start_index;        // Index of first visible process
    int end_index;
    int width;			//evtl
	
    int selected = 0;     // Index in processes

public:
    Scrollable_Table(int y, int display_rows);


    void set_processes(std::vector<std::unique_ptr<Process>> new_processes); // todo
    std::unique_ptr<Process> remove_process_selected();
    void add_process(std::unique_ptr<Process> process);

    void selected_up();
    void selected_down();
    void update_display_rows(int new_display_rows);
    int get_display_rows() const;			// todo
    int get_process_count() const;			// todo

    void draw(const char* title);

    // For manager
    const std::vector<std::unique_ptr<Process>>& get_processes() const;
};

#endif
