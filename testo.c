#include <stdio.h>

// Libreria utilizzata per configurare e controllare le linee di comunicazione seriali e i terminali
#include <termios.h>
#include <unistd.h>

// Questa struct definisce lo stato di configurazione del terminale
// (flag di input, output, controllo)
struct termios origin_raw;

// Disactivation of the Raw Mode in the terminal
void disableRawMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_raw);
}

// Activation of the Raw Mode in the terminal
void enableRawMode() {

	// Legge gli attributi correnti associati allo Standard Input
	// Necessario farlo ogni volta che si vuole modiifcare n-parametri dello Standard Input
	tcgetattr(STDIN_FILENO, &origin_raw);
	atexit(disableRawMode);

	struct termios raw = &origin_raw;

	// Modifica del flag di input c_lflag (local flags)
	// Uno di questi è il flag ECHO, quando ECHO = 1 , ogni lettera digitata viene stampata a video
	// l'operatore '&=0 azzera unicamente quel bit nessun carattere a schermo
	raw.c_lflag &= ~(ECHO);

	// Riapplicazione dei parametri al terminale
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {

	enableRawMode();

    char c;
    while( (read(STDIN_FILENO, &c, 1) == 1) && (c != 'q') );

    return 0;

}