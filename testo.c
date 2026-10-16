#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>


struct termios origin_raw;

#define CTRL_KEY(k) ((k) & 0x1f)


/*
* Disegno tilde 
*/
void drawRow() {
	int y;

	for (y=0; y<48; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}


void refreshScreen() {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	drawRow();

	write(STDOUT_FILENO, "\x1b[H", 3);
}

void die(const char *s) {
	refreshScreen();
	perror(s);
	exit(1);
}

void disableRawMode() {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_raw) == -1) {
		die("tcsetattr");
	}
}

void enableRawMode() {

	if (tcgetattr(STDIN_FILENO, &origin_raw) == -1) {
		die("tcgetattr");
	}

    atexit(disableRawMode);

    struct termios raw = origin_raw;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
		die("tcsetattr");
	}
}

char readKey() {
	int nread;
	char c = '\0';

	nread = read(STDIN_FILENO, &c, 1);
	if (nread == -1) {
		die("read");
	}

	return c;
} 

void processReadKey() {
	char c = readKey();

	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}

int main() {

    enableRawMode();

    while (1) {
		refreshScreen();
		processReadKey();
	}

    return 0; 
}