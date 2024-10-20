// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <memory>  // for allocator, __shared_ptr_access, shared_ptr
#include <string>  // for to_string, operator+
#include "include/ftxui/dom/table.hpp"
#include "include/ftxui/component/captured_mouse.hpp"  // for ftxui
#include "include/ftxui/component/component.hpp"//? for Button, Renderer, Vertical
#include "include/ftxui/component/component_base.hpp"  // for ComponentBase
#include "include/ftxui/component/component_options.hpp"   // for ButtonOption
#include "include/ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "include/ftxui/dom/elements.hpp"  // for operator|, text, Element, hbox, separator, size, vbox, border, frame, vscroll_indicator, HEIGHT, LESS_THAN
#include "include/ftxui/screen/color.hpp"  // for Color, Color::Default, Color::GrayDark, Color::White
 
using namespace ftxui;

Component MakeOne() {
  class Impl : public ComponentBase {
    Element Render() final {
      // Create a table row with elements and add padding between each.
      Element table_row = hbox({
        text("This")	| flex,
	separator()	| flex,
	text("is")	| flex, 
	separator()	| flex, 
	text("a")	| flex, 
	separator()	| flex, 
	text("test")	| flex,
      });

      // Apply focus and selection styles
      if (Focused()) {
        return focus(table_row | inverted) | flex;  // Invert the colors when focused
      } else if (Active()) {
        return select(table_row | inverted) | flex; // Invert and show as selected when active
      } else {
        return table_row | flex;  // Normal row
      }
    }

    bool Focusable() const final { return true; }
  };

  return Make<Impl>();
}




int main() {
  // Create a vertical container
  auto container = Container::Vertical({});

  // Add multiple rows (for instance, 30 rows)
  for (int i = 0; i < 30; i++) {
    container->Add(MakeOne());
  }

  // Create the interactive screen
  auto screen = ScreenInteractive::FitComponent();

  // Renderer for the container inside a window
  auto renderer = Renderer(container, [&] {
    // Create a window that contains the table
    auto window_content = container->Render()
                          | vscroll_indicator  // Scrollbar for the table
                          | frame              // Enable scrolling within the frame
                          | flex;              // Stretch the container vertically and horizontally

    // Wrap the content in a window with a border
    return window(text("Bashfreeze"), window_content) | flex;
  });

  // Handle exit event on 'q' key press
  auto component = CatchEvent(renderer, [&](Event event) {
    if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }
    return false;
  });

  screen.Loop(component);
}

