# 1 Project Introduction
La realizzazione di questo paper risulta per me fondamentale: serve tanto a me quanto a chiunque altro sarà interessato a comprendere le logiche che vi sono dietro a Testo e a chi mai vorrà contribuire a portare avanti il progetto. Quello che segue rappresenta per me il primo documento di testo tecnico, nonchè la prima volta che un mio progetto risulta avere un documento così profondo. Il nome che ho affidato a questo file è decisamente azzeccato: più che un documento tecnico, rappresenta la traccia del mio passaggio. 

## 1.1 What is Testo?
Prima ancora di addentrarci nelle logiche da implementare e dai temi da scoprire durante lo sviluppo, credo sia altrattanto fondamentale definire in parole quello che voglio che diventi **Testo**. 

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
Come possiamo replicare questa modalità? Prima ancora di capire il come, illustro gli strumenti che abbiamo a disposizione per manipolare il terminale. I primi strumnenti arrivano dalla libreria:

``` c
#include <termios.h>
```

Questa libreria fornisce in particolare una `struct` che permette di manipolare la configurazione del terminale, mediante l'attvazione di alcuni flag. Per prima cosa allora abbiamo definito una struct globale per mantenere lo stato orginale del terminale. 

``` c
struct termios original_raw;
```

Il ragionamento è semplice la prima cosa che dobbiamo fare è definire le logche di attivazione/disattivazione della **Raw Mode**. Per fare questo abbiamo definito due metodi: il `enableRawMode` ed `disableRawMode`;

Il metodo `enableRawMode` utilizza un metodo proveniente da un'altra libreria: 

``` c
#include <unistd.h>
```


