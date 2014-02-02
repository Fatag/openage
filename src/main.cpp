#include "openage/main.h"
#include "engine/log.h"
#include "engine/util/error.h"

#include "engine/console/buf.h"
using namespace engine::console;
using namespace engine::coord;

#include <unistd.h>
#include <termios.h>
struct termios old_tio, new_tio;
void setstdincanon() {
	if (isatty(STDIN_FILENO)) {
		//get the terminal settings for stdin
		tcgetattr (STDIN_FILENO, &old_tio);
		//backup old settings
		new_tio = old_tio;
		//disable buffered i/o (canonical mode) and local echo
		new_tio.c_lflag &= (~ICANON & ~ECHO & ~ISIG);
		//set the settings
		tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
	}
}

void restorestdin() {
	if (isatty(STDIN_FILENO)) {
		tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
	}
}

void test0() {
	Buf buf{{80, 25}, 1337, 80};
	buf.write("Hello, brave new console world!\n\n\n\n");
	buf.write("stuff, lol.\n\n");
	buf.write("\x1b[1mbold stuff, lol.\x1b[m\n\n");
	buf.write("\x1b[5;31;1mred bold blinking stuff, lol, ... also, this text seems to be quite long, maybe even wider than the terminal width. i wonder what could \x1b[7mhappen.\x1b[m\n\n");
	for (int i = 0; i < 18; i++) {
		buf.write("asdf\n");
	}
	buf.scroll(100);
	buf.to_stdout(true);
}

void test1() {
	//don't echo input, unbuffered input
	setstdincanon();
	//hide cursor
	printf("\x1b[?25l");
	Buf buf{{80, 25}, 1337, 80};
	buf.to_stdout(true);
	while (true) {
		char c;
		if (read(0, &c, 1) != 1) {
			printf("EOF\n");
			break;
		}
		if (c == 3) {
			printf("^C\n");
			break;
		}
		if (c == 4) {
			printf("^D\n");
			break;
		}
		if (c == 1) {
			//^A: scroll 1 up
			buf.scroll(1);
		}
		if (c == 2) {
			//^B: scroll 1 down
			buf.scroll(-1);
		}
		buf.write(c);
		//clear buf from current cursorpos
		printf("\x1b[J");
		buf.to_stdout(false);
	}
	//show cursor
	printf("\x1b[?25h");
	restorestdin();
}

int main() {
	test1();
	return 0;

	try {
		return openage::main();
	} catch (engine::Error e) {
		engine::log::fatal("Exception: %s", e.str());
		return 1;
	}
}