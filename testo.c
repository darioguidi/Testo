#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

// Libreria utilizzata per configurare e controllare le linee di comunicazione seriali e i terminali
#include <termios.h>

// Fornisce l'accesso alle API POSIX di sistema (read, write, STDIN_FILENO)
#include <unistd.h>

// Questa struct memorizza lo stato originale del terminale (flag di input, output, controllo, caratteri locali)
// Viene dichiarata globale per poter essere consultata da disableRawMode() al termine del programma
struct termios origin_raw;

// Ripristina lo stato originale del terminale
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_raw);
}

// Configura il terminale in modalità non-canonica / raw
void enableRawMode() {

    // Legge gli attributi correnti associati allo Standard Input e li salva in origin_raw
    // È necessario farlo prima di qualsiasi modifica per preservare la configurazione di partenza
    tcgetattr(STDIN_FILENO, &origin_raw);

    // Registra disableRawMode come callback di uscita nel runtime C.
    // Verrà eseguita in automatico alla terminazione del programma
    // assicurando che il terminale non rimanga "rotto" (senza echo) dopo la chiusura
    atexit(disableRawMode);

    struct termios raw = origin_raw;

    // Modifica del campo c_lflag (Local Flags):
    // Il flag ECHO controlla la stampa immediata a video dei tasti premuti.
    // L'operatore bitwise '&=' combinato con '~ECHO' (NOT bit a bit) azzera esclusivamente
    // il bit di ECHO senza alterare gli altri flag presenti
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // Riapplica la nuova configurazione modificata (raw) allo standard input del terminale
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {

    // Inizializza il terminale escludendo l'echo a schermo
    enableRawMode();

    char c;

    // Loop di lettura a basso livello:
    // - read() legge fino a 1 byte alla volta direttamente dallo STDIN_FILENO nel buffer 'c'.
    // - Ritorna il numero di byte letti (1 in caso di successo, 0 a EOF, -1 in caso di errore).
    // - Il ciclo continua finché l'utente non digita il carattere 'q'
    while ((read(STDIN_FILENO, &c, 1) == 1) && (c != 'q')) {
		if (iscntrl(c)) {
			printf("%d\r\n", c);
		} else {
			printf("%d ('%c')\r\n", c, c);
		}
	}
	

    return 0; 
}