#include "class_Process.h"
#include "include/ftxui/component/component.hpp"
#include "include/ftxui/component/component_options.hpp"
#include "include/ftxui/dom/elements.hpp"
#include "include/ftxui/screen/color.hpp"
#include "include/ftxui/component/captured_mouse.hpp"
#include "include/ftxui/component/screen_interactive.hpp"
#include <memory>
#include <string>
#include <vector>


using namespace ftxui;


static Component createElement(std::string process) {
	class Impl : public ComponentBase {
			Element Render() final { 
			auto element = text("Hier muss der geparste Text rein");

			if (Focused()) {
				element = focus(element);
				element |= inverted;
			      } else if (Active()) {
				element = select(element);
				element |= inverted;
			      }

		      return element;
	    }

	    bool Focusable() const final { return true; }
  };

  return Make<Impl>();
}

static std::string parse_process(std::unique_ptr(Process) process) {
	format





}


Component CreateScrollableTable(const std::vector<std::string> elements, int* selected_element) {
	//create vertical container that holds all items
	auto container = Container::Vertical({});

	for(int i = 0; i < elements.size(); i++) {
		container->Add(createElement(std::string process));
	}
	auto renderer = Renderer(container, [&] {
		return container->Render()
			| vscroll_indicator
			| frame
			| size(HEIGHT, LESS_THAN, 20)
			| border;
	});
	return renderer;
}
