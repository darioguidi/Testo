/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/
// Memorizza lo stato originale del terminale
struct termios origin_raw;

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** terminal ***/
void refreshScreen() {
	write(STDOUT_FILENO, "\x1b[2j", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
}

void die(const char *s) {
	refreshScreen();
	perror(s);
	exit(1);
}

// Ripristina lo stato originale del terminale
void disableRawMode() {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_raw) == -1) {
		die("tcsetattr");
	}
}

// Configura il terminale in modalità non-canonica / raw
void enableRawMode() {

    // Legge gli attributi correnti associati allo Standard Input e li salva in origin_raw
    // È necessario farlo prima di qualsiasi modifica per preservare la configurazione di partenza
    if (tcgetattr(STDIN_FILENO, &origin_raw) == -1) {
		die("tcgetattr");
	}

    // Registra disableRawMode come callback di uscita nel runtime C.
    atexit(disableRawMode);

    struct termios raw = origin_raw;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

    // Riapplica la nuova configurazione modificata (raw) allo standard input del terminale
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
			readKey();
			exit(0);
			break;
	}
}

/*** init ***/
int main() {

    enableRawMode();

    /* read() legge fino a 1 byte alla volta direttamente dallo STDIN_FILENO nel buffer 'c'.
     * Ritorna il numero di byte letti (1 in caso di successo, 0 a EOF, -1 in caso di errore).
     * Il ciclo continua finché l'utente non digita il carattere 'q'
	 */
    while (1) {
		refreshScreen();
		processReadKey();
	}
	

    return 0; 
}