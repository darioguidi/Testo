# 1 Project Introduction
La realizzazione di questo paper risulta per me fondamentale: serve tanto a me quanto a chiunque altro sarà interessato a comprendere le logiche che vi sono dietro a Testo e a chi mai vorrà contribuire a portare avanti il progetto. Quello che segue rappresenta per me il primo documento di testo tecnico, nonché la prima volta che un mio progetto risulta avere un documento così profondo. Il nome che ho affidato a questo file è decisamente azzeccato: più che un semplice documento tecnico, rappresenta la traccia del mio passaggio.

## 1.1 What is Testo?
Prima ancora di addentrarci nelle logiche da implementare e nei temi da scoprire durante lo sviluppo, credo sia altrettanto fondamentale definire a parole quello che voglio che diventi **Testo**.

Da cosa nasce l'idea di **Testo**? In parole povere desideravo realizzare qualcosa di mio, essendo veramente stanco di consumare solo quello che mi veniva propinato. In particolare desideravo realizzare qualcosa di funzionale: tutti i progetti precedenti avevano come unico scopo la mia personale soddisfazione, mentre questo comprenderebbe un reale, seppur ormai replicato e superato, fine.

# 2 Requirements
La parte dei requirement è facilmente descrivibile dal file presente in repo, chiamato `requirements.txt`.

*SEZIONE DA COMPLETARE / DEFINIRE*

# 3 Development
## 3.1 Why Enter the Raw Mode
Il primo tema che mi sono ritrovato a studiare durante lo sviluppo è quello inerente alle modalità presenti nel terminale. Dobbiamo quindi affrontare il primo elefante in questo negozio di porcellana, ovvero comprendere cosa sia la **Raw Mode**.

Per riuscire in questo, trovo molto utile impegnarmi a comprendere *tutte* le modalità descrivibili in terminale. Parliamo dunque della **Cooked Mode** e della **Raw Mode**.

Normalmente tra la keyboard ed il PC è presente un componente del kernel chiamato **Line Discipline**. Esso trattiene tutti gli input forniti da keyboard, presenti come byte, in un buffer e permette una serie di cose: in particolare mostra a schermo i byte nel loro corrispettivo *char*, permette di spostarsi avanti e indietro con le freccette e di cancellare con *backspace* questi byte dal buffer.

La **Cooked Mode** ha questo componente attivato, mentre la **Raw Mode** lo disattiva. Con le informazioni raccolte possiamo quindi descrivere e dedurre una serie di comportamenti che, per raggiungere il nostro fine, ovvero realizzare un text editor in bash, risulta di fondamentale importanza identificare.

La **Cooked Mode** ha la particolarità di raccogliere gli input da keyboard, mostrarli a schermo nel loro corrispettivo *char* e aspettare il comando *Enter* per eseguire quanto scritto. Le combinazioni di tasti CTRL+? hanno un loro significato, come CTRL+C (kill del processo) e CTRL+Z (sospensione del processo). L'invio a capo viene tradotto come `\r\n`, andando a capo e a inizio riga.

D'altra parte la **Raw Mode** si comporta in modo del tutto differente. Innanzitutto questa legge gli input nel momento in cui vengono inseriti e non aspetta l'invio da parte dell'utente. Le combinazioni CTRL+? vengono lette come byte. L'invio a capo viene letto semplicemente come `\n`.

Ora che si è capito esattamente cosa sia la Raw Mode possiamo rispondere alla domanda posta come titolo. Dobbiamo necessariamente entrare in questa modalità proprio per simulare il comportamento di un normale text editor: abbiamo bisogno di una lettura immediata dei char di input e non di attendere un dato input per la lettura del terminale.

## 3.2 How to Enter the Raw Mode
Come possiamo replicare questa modalità? Prima ancora di capire il come, illustro gli strumenti che abbiamo a disposizione per manipolare il terminale. I primi strumenti arrivano dalla libreria:

``` c
#include <termios.h>
```

Questa libreria fornisce in particolare una `struct` che permette di manipolare la configurazione del terminale, mediante l'attivazione di alcuni flag. Per prima cosa abbiamo definito una struct globale per mantenere lo stato originale del terminale.

``` c
struct termios origin_raw;
```

Il ragionamento è semplice: definire le logiche di attivazione e disattivazione della **Raw Mode**. Per fare questo abbiamo definito due metodi: `enableRawMode` e `disableRawMode`.

### Enable Raw Mode
Il metodo `enableRawMode` utilizza un metodo proveniente da un'altra libreria:

``` c
void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &origin_raw) == -1) {
		die("tcgetattr");
	}
	atexit(disableRawMode);
	...
}
```

### What is the function `tcgetattr()`
Il metodo introdotto è `tcgetattr()` della libreria `termios.h`. Questo permette di copiare la configurazione di default del terminale, composta da diversi flag, relativa allo standard input `STDIN_FILENO`, nella struct termios definita a livello globale. Ora abbiamo un "backup" della situazione di partenza.

### What is the function `exit()`
La funzione `exit()` fa parte della seguente libreria:

``` c
#include <stdlib.h>
```

Questa interrompe l'esecuzione del programma in qualsiasi punto del programma in cui esso si trovi. La firma è la seguente:

``` c
void exit(int status);
```

La funzione accetta due stati:
- `EXIT_SUCCESS`, che corrisponde a `0`, indica che il programma si conclude senza errori.
- `EXIT_FAILURE`, che corrisponde solitamente a `1`, indica che il programma si conclude con errori.

Va specificato che la funzione `exit()` non conclude l'esecuzione del programma in maniera brusca, ma effettua un **clean-up**:
- **Esegue tutte le funzioni registrate con `atexit()`**: vengono eseguite tutte le funzioni registrate. Noi abbiamo già registrato la funzione di disattivazione della raw mode.
- **Svuota i buffer di I/O**
- **Chiude tutti gli stream di file aperti**
- **Rimuove i file temporanei**: rimozione di tutti i file creati con la funzione `tmpfile()`.

### Error Handling
Il metodo `tcgetattr()` in caso di errore restituisce `-1`. Ho creato un metodo di utility `die()` che, una volta invocato, deve fare il refresh dello schermo e invocare la `exit(1)`.

``` c
void refreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4); // Invia la sequenza di escape ANSI (4 byte) per cancellare tutto il contenuto visibile dello schermo (terminale)
    write(STDOUT_FILENO, "\x1b[H", 3);  // Invia la sequenza di escape ANSI (3 byte) per riposizionare il cursore in alto a sinistra (riga 1, colonna 1)
}

void die(const char *s) {
	refreshScreen();
	perror(s);
	exit(1);
}
```

### Terminal Configuration
In realtà attivare la **Raw Mode** consiste nella manipolazione di tutti quei flag di configurazione presenti nella struct termios. Creiamo una struct partendo da quella globale:

``` c
struct termios raw = origin_raw;

raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
raw.c_oflag &= ~(OPOST);
raw.c_cflag |= (CS8);
raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
raw.c_cc[VMIN] = 0;
raw.c_cc[VTIME] = 1;
```

### What is the function `tcsetattr()`
Abbiamo bisogno ora di applicare questo set di flag, dunque questa configurazione, al terminale. La funzione `tcsetattr()` della libreria `termios.h` fa proprio questo:

``` c
if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
	die("tcsetattr");
}
```

### Disable Raw Mode
Per disattivare la raw mode abbiamo dunque bisogno di ripristinare la configurazione di partenza del terminale:

``` c
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_raw) == -1) {
		die("tcsetattr");
	}
}
```

## 3.3 Writing Text Logic
Ora possiamo ragionare su come implementare tutta la logica che vi è dietro la scrittura all'interno del text editor.

``` c
int main() {

    enableRawMode();

	while (1) {
		refreshScreen();
		processReadKey();
	}

    return 0;
}
```

Serve creare una funzione che si occupi dell'acquisizione del char passato in input e della gestione delle varie *special key* (quali tutte le combinazioni formate da CTRL+K). Questa è formata da due parti: acquisizione dell'input e gestione delle varie casistiche.

``` c
#define CTRL_KEY(k) ((k) & 0x1f)

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
```

### What is the function `read()`
Si tratta di una funzione appartenente alla libreria `<unistd.h>` che permette la lettura di byte e il loro salvataggio in un buffer di memoria.

``` c
ssize_t read(int fd, void *buf, size_t count);
```

- `fd`: indica il file descriptor dal quale leggere; in questo caso passiamo `STDIN_FILENO`, che vale `0` e rappresenta lo standard input da tastiera.
- `*buf`: è il puntatore all'area di memoria dove verranno salvati i byte.
- `count`: indica il numero massimo di byte da salvare in una singola chiamata. Per rappresentare un char basta 1 byte.
- return `ssize_t`: ritorna il numero di byte letti.

### What is the function `write()`
Si tratta di una funzione appartenente alla libreria `<unistd.h>` che permette la scrittura di byte. Segue la firma della funzione:

``` c
ssize_t write(int fileDescriptor, const void *buffer, size_t bytesToWrite);
```

- `fileDescriptor`: indica il file descriptor sul quale scrivere; in questo caso passiamo `STDOUT_FILENO`, che vale `1` e rappresenta lo standard output (lo schermo del terminale). Questo `int` viene anche restituito dalla funzione `open()` quando vogliamo aprireun file locale.
- `*buffer`: è il puntatore all'area di memoria dove sono salvati i byte da scrivere.
- `bytesToWrite`: indica il numero massimo di byte da scrivere in una singola chiamata. Per rappresentare un char basta 1 byte.
- return `ssize_t`: ritorna il numero di byte scritti.

## Draw Tildes

``` c 
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
```

## Measure Window Size
Per misurare le dimensioni della finestra da terminale ricorreremo ad alcune funzioni contenute nella seguente libreria:

``` c
<sys/ioctl.h>
```

### struct `winsize`
Si tratta di una struttura in grado di salvare al suo interno le dimensioni di una determinata finestra. Essa sarebbe: 

``` c 
struct winsize {
    unsigned short ws_row;    /* rows, in characters */
    unsigned short ws_col;    /* columns, in characters */
    unsigned short ws_xpixel; /* horizontal size, pixels (unused/0 in most terminals) */
    unsigned short ws_ypixel; /* vertical size, pixels (unused/0 in most terminals) */
};
```

### What is the function `ioctl()`
Si tratta della funzione in grado di salvare le dimensioni correnti della finestra del terminale (righe, colonne e pixel) all'interno di una struttura dati struct winsize, interrogando direttamente il driver del terminale nel kernel tramite la richiesta `TIOCGWINSZ` (Terminal Input Output Control Get Window Size).

``` c 
int ioctl(int fileDescriptor, unsigned long request, struct winsize); 
```

## The Hard Way To Mesure Windows Size
Il problema che emerge è quello per cui, seppur fighissima, la funzione `ioctl()` potrebbe non funzionare su tutti i sistemi. Quindi, la *hard way* consiste in un calcolo esatto mediante il movimento del cursore delle righe e delle colonne.


## Last Line Bug
Il bug che introdotto riguarda la scrittura dei tilde in terminale. Il ciclo opera sull'intero ammontare di righe disponibili. Una volt arrivati all'ultima riga, quello che viene stampato rimane sempre `~\r\n`, dunque pur di eseguire tutti i comandi, il programma creava una new blank line di troppo. Ora questa fa eccezione.