#ifndef SCROLLABLE_TABLE_H
#define SCROLLABLE_TABLE_H

#include <vector>
#include <memory>
#include <ncurses.h>

class Process; // Forward declaration for Process class

class Scrollable_Table {
public:
    Scrollable_Table(int rows, int columns, int position_x, int position_y);
    Scrollable_Table(std::vector<std::unique_ptr<Process>> processes, int rows, int columns, int position_x, int position_y);

    void set_processes(std::vector<std::unique_ptr<Process>> processes);
    void set_window_dimensions(int rows, int columns);
    void set_window_position(int x, int y);
    void set_selected(int selected);

    void selected_increment();
    void selected_decement();

    int get_input();
    void draw_table();

private:
    WINDOW *win;
    std::vector<std::unique_ptr<Process>> processes;
    int rows;
    int columns;
    int start;
    int end;
    int selected;
    int position_x;
    int position_y;
};

#endif // SCROLLABLE_TABLE_H
