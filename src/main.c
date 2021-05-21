#include "gui.h"

int main() {
	
	struct gui_state gui;
	
	SDL_Event e;
	if(gui_init(&gui, 1000, 800)) {
		while(1) {
			SDL_PollEvent(&e);
			
			gui_handle_event(&gui, &e);
			gui_update(&gui);
		}

		gui_close(&gui);
	}

	return 0;
}

