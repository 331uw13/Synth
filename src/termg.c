#include <stddef.h>
//#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
//#include <string.h>
#include <sys/ioctl.h>
#include <stdarg.h>

#include "termg.h"


static struct termios termios_old;
static u16 termg_fg_color;
static u16 termg_bg_color;


void termg_init() {

	printf("\033[?25l");
	// Save current termios mode.
	tcgetattr(STDIN_FILENO, &termios_old);

	// TODO: setup signal handler for resizing.

	// Enable raw mode.
	struct termios raw;
	cfmakeraw(&raw);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);

	termg_color(TERMG_BG, TERMG_BLACK);
	termg_color(TERMG_FG, TERMG_WHITE);
	termg_clear();
}


void termg_quit() {
	// Restore termios mode.
	tcsetattr(STDIN_FILENO, TCSANOW, &termios_old);
	printf("\033[?25h");
}


void termg_win_size(u16* w, u16* h) {
	if(w == NULL && h == NULL) { return; }

	struct winsize b;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &b);

	if(w != NULL) {
		*w = b.ws_col;
	}
	if(h != NULL) {
		*h = b.ws_row;
	}
}


void termg_clear() {
	printf(
			"\033[48;5;%im"
			"\033[2J"
			"\033[H",
			termg_bg_color
			);
}


void termg_clear_line(u16 y) {
	printf(
			"\033[48;5;%im"
			"\033[%i;0H"
			"\033[K",
			termg_bg_color,
			y+1
			);

}


void termg_color(u8 flag, u16 color) {
	if(flag == TERMG_FG) {
		termg_fg_color = color;
	}
	else if(flag == TERMG_BG) {
		termg_bg_color = color;
	}
}


void termg_print(u16 x, u16 y, const char* str, ...) {

	char buffer[TERMG_PRNTBUFSIZE];

	va_list ap;
	va_start(ap, str);
	vsnprintf(buffer, TERMG_PRNTBUFSIZE, str, ap);
	va_end(ap);
	
	printf( "\033[%i;%iH"
			"\033[48;5;%im"
			"\033[38;5;%im"
			"%s"
			"\033[0m",
			y+1, x+1,
			termg_bg_color,
			termg_fg_color,
			buffer
			);

}





