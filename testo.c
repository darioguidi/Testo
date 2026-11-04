#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

// Stato originale del terminale
struct editorConfig {
	int screencols;
	int screenrows;
	struct termios origin_raw;
}
struct editorConfig E;

#define CTRL_KEY(k) ((k) & 0x1f)
#define LENGHT_COLUMN_TILDE 48

/*
 * Calcola le dimensioni della finestra da terminale
 * IOCTL = Input Output Control
 * request al kernel TIOCTRLWINS = Terminal Input Output Control Window Size 
 */
int getWindowSize(int *col, int *rows) {
	struct winsize ws;

	if ( (ioctl(STDOUT_FILENO, TIOCTLWINSZ, &ws) == -1) && (ws.ws_col == 0) ) {
		die("Errore lettura window size");
		return -1;
	} else {
		*col = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}


void initEditor() {
	getWindowSize(&E.screencols, &E.screenrows);
}


/* 
* Scrive un tilde, il char di a capo e il char di new line. 
* Per farlo utilizza la funzione write() direttamente sullo standard di output (terminale)
* Il numero esatto di scrittura e determinato dalla macro LENGHT_COLUMN_TILDE
*/
void drawRow() {
	for (int y=0; y < LENGHT_COLUMN_TILDE; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}


/*
* La prima operazione che effettua è una write, sul STDOUT_FILENO per pulire lo schermo
* la seconda operazione è un'altra write per posizionare il cursore a inizio schermo
* chiama drawRow() per disegnare i tildes
* riposiziona il cursore affianco il primo tilde
*/
void refreshScreen() {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	drawRow();

	write(STDOUT_FILENO, "\x1b[H", 3);
}


/*
* In caso di qualsiasi errore si chiama questa funzione
* esegue refresh dello screen chiama la exit(), il programma viene interrotto
*/
void die(const char *s) {
	refreshScreen();
	perror(s);
	exit(1);
}


/*
* Lettura di un char
*/
char readKey() {
	int nread;
	char c = '\0';

	nread = read(STDIN_FILENO, &c, 1);
	if (nread == -1) {
		die("read");
	}

	return c;
} 


/*
* Lettura di un char e process delle special key
* usando macro CTRL_KEY
*/
void processReadKey() {
	char c = readKey();

	switch (c) {

		// quit program
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}


int main() {

    enableRawMode();
	initEditor();

    while (1) {
		refreshScreen();
		processReadKey();
	}

    return 0; 
}