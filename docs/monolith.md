# 1 Project Introduction
La realizzazione di questo paper risulta per me fondamentale: serve tanto a me quanto a chiunque altro sarà interessato a comprendere le logiche che vi sono dietro a Testo e per chi mai vorrà contribuire a portare avanti il progetto. Quello che segue rappresenta per me il primo documento di testo tecnico, nonchè la prima volta che un mio progetto risulta avere un documento così profondo. Il nome che ho affidato a questo file è decisamente azzeccato: più che un semplice documento tecnico, rappresenta la traccia del mio passaggio. 

## 1.1 What is Testo?
Prima ancora di addentrarci nelle logiche da implementare e dai temi da scoprire durante lo sviluppo, credo sia altrattanto fondamentale definire in parole quello che voglio diventi **Testo**. 

Da cosa nasce l'idea di **Testo**? In parole povere desideravo realizzare qualcosa di mio, essendo veramente stanco di consumare solo quello che mi veniva propinato. In particolare desideravo realizzare qualcosa di funzionale: tutti i progetti precedenti avevano come unico scopo la mia personale soddisfazione, mentre questo coprenderebbe un reale, seppur ormai replicato e superato, fine.  

# 2 Requirements
La parte dei requirement e facilmente descrivibile dal file presente in repo, chiamato `requirements.txt`.

*SEZIONE DA COMPLETARE / DEFINIRE*

# 3 Development 
## 3.1 Why Entering The Raw Mode
Il primo tema che mi sono ritrovato a studiare durante lo sviluppo è quello inerente le modalità presenti nel terminale. Dobbiamo quindi affrontare il primo elefante in questo negozio di porcellana, ovvero comprendere cosa sia la **Raw Mode**.

Per riuscire in questo, trovo molto utile impegnarmi a comprendere *tutte* le modalità descrivibili in terminale. Parliamo dunque della **Cooked Mode** e della **Raw Mode**.

Normalmente tra la keyboard ed il PC è presente un componente del kernel chiamato **Line Discipline**. Esso trattiene tutti gli input forniti da keyboard, presenti come byte, in un buffer e permette una serie di cose, in particolare mostra a schermo i byte nel loro corrispettivo *char*, permette di spostarsi avanti e indietro con le freccette e con *backspace* cancellare questi byte dal buffer. 

La **Cooked Mode** ha questo componente attivato, mentre la **Raw Mode** lo disattiva. Con le informazioni raccolte possiamo quindi descrivere e dedurre una serie di comportamenti, che per raggiungere il nostro fine, ovvero realizzare un text edito in bash risultano di fondamnetale importanza identificare.

La **Cooked Mode** ha la particolarità che raccoglie gli input da keyboard, li mostra a schermo nel loro corrispettivo *char* e aspetta il comando *Enter* per eseguire quanto scritto. Le combinazione di tasti CTRL+? hanno un loro significato, come il CTRL+C (Kill del processo) e il CTRL+Z (Sospensione del processo). L'invio a capo viene tradotto come `\r\n` andando a capo e inizio riga.

D'altra parte la **Raw Mode** si comporta in modo del tutto differente. Inanzitutto questa legge gli input nel momento in cui questi vengono inseriti e non aspetta l'invio da utente. Le combinazioni CTRL+? vengono letyte come byte. L'invio a capo viene letto semplicemente come `\n`. 

Ora che si è capito eattamente cosa sia la Raw Mode possiamo rispondere alla domanda posta come titolo. Dobbiamo necessariamente entrare in questa modalità proprio per simulare il comportamento di un normale text editor, dunque abbiamo bisogno di una lettura dei char di input e non ti aspettare un dato input per la letturadel terminale.

## 3.2 How Entering the Raw Mode
Come possiamo replicare questa modalità? Prima ancora di capire il come, illustro gli strumenti che abbiamo a disposizione per manipolare il terminale. I primi strumenti arrivano dalla libreria:

``` c
#include <termios.h>
```

Questa libreria fornisce in particolare una `struct` che permette di manipolare la configurazione del terminale, mediante l'attivazione di alcuni flag. Per prima cosa abbiamo definito una struct globale per mantenere lo stato orginale del terminale. 

``` c
struct termios original_raw;
```

Il ragionamento è semplice: definire le logiche di attivazione e disattivazione della **Raw Mode**. Per fare questo abbiamo definito due metodi: `enableRawMode` ed `disableRawMode`;


### 3.2.1 Enable Raw Mode
Il metodo `enableRawMode` utilizza un metodo proveniente da un'altra libreria: 

``` c
#include <unistd.h>

void enableRawMode() {
	if (tcgetattr(STDLIN_NO, &origin_raw) == -1) {
		die("tcgetattr");
	}

	atexit(disableRawMode);

	...
}
```

### What is the function `tcgetattr()` 
Il metodo introddoto si tratta essere `tcgetattr()`. Questo permette di copiare la configurazione default del terminale, composta da diversi flag, nella struct termios, definita a livello globale. Attualmente abbiamo un "backup" della situazione di partenza. 

### What is the function `exit()`
La funzione `exit()` fa parte della seguente libreria:

``` c
#include <stdlib.h>
```

Questa interompe l'esecuzione del programma in qalsiasi punto del programma nella quale esso sia. La firma è la seguente:

``` c
void exit(int status);
```

La funzione può resituire due stati: 
- `EXIT_SUCCESS` che corrisponde a `0` indica che il programma si conclude con nessun errore.
- `EXIT_FAILURE` che corrisponde solitamente a `1`, indica che il programma si conclude con errori. 

Va specificato che la funzione `exit()` non conclude l'esecuzione del programma in maniera brusca, ma effettua un **clean-up**:
- **Esegue tutte le funzioni registrate nella `atexit()`**: vengono eseguite tutte le funzioni registrate. Noi abiamo già registrato la funzione di disattivazione della raw mode.
- **Svuota i buffer di I/O**
- **Chiude tutti gli stream di file aperti**
- **Rimuove i file temporanei**: Rimozione di tutti i file creati con la funzione `tmpfile()`.

### Error Handle
Il metodo `tcgetattr()` in caso di errore restituisce `-1`. Ho creato un metodo di utility `die()`, che una volta invocato deve fare refresh dello schermo e invocare la `exit(1)`;

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
Nella realtà attivare la **Raw Mode** consiste nella manipolazione di tutti quei flag di configurazione presenti nella strcut termios. Creiamo una struct partendo da quella globale:

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
Abbiamo bisogno ora di applicare questo set di flag, dunque questa configurazione al terminale. La funzione fa proprio questo:

``` c
if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
	die("tcsetattr");
}
```

### 3.2.2 Disable Raw Mode
Per disattivare la raw mode abbiamo dunque bisogno di ripristinare la configurazione di partenzadel terminale:

``` c
void disableRawMode() {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_raw) == -1) {
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

Serve creare una funzione che si occupi della acquisizione del char passato in input e della gestione dei vari *special key* (quali tutte le combinazioni formate da CTRL+K). Questa è formata da due parti: acquisizione del input e gestione delle varie casistiche. 

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
Si tratta di una funzione appartenente alla libreria `<unistd.h>` che permette la lettura di byte e la loro allocazione in un buffer di memoria.

``` c
ssize_t read(int fd, void *buf, size_t count);
```

- `fd` : Indica il file descriptor dal quale leggere

### What is the function `write()`
Si tratta di una funzione appartenente alla libreria `<unistd.h>` che permette la lettura del singolo char. Segue la firma della funzione:

``` c
ssize_t write(int fd, const void *buf, size_t count);
```

### Draw Tildes

### Neasure Window Size