#include "include/ftxui/dom/canvas.hpp"
#include "include/ftxui/dom/elements.hpp"
#include "include/ftxui/screen/screen.hpp"
#include "include/ftxui/component/screen_interactive.hpp"
#include "include/ftxui/component/component.hpp"
#include <cstdlib>

int main(void) {
	using namespace ftxui;
	auto screen = ScreenInteractive::TerminalOutput();
		std::vector<std::string> entries = {
		    "entry 1",
		    "entry 2",
		    "entry 3",
		};
	int selected = 0;
	auto menu = Radiobox({
	  .entries = entries,
	  .selected = &selected,
	});
	screen.Loop(menu);
	return EXIT_SUCCESS;




}
