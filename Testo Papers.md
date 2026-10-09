# Entering The Raw Mode
Subito dopo aver completato tutti i passi necessari per un setup funzionante e prima ancora di iniziare con lo sviluppo vero e proprio di Testo, mi sono chiesto cosa significa "entrare in Raw Mode", in quanto si tratta di un termine a me ancora sconosciuto.

Di seguito spiego la differenza tra la Canonical Mode e la Raw Mode:
- **Canonical Mode**: Il terminale in questa modalità legge *for line*. L'utente scrive sulla riga i comandi che vuole eseguire oppure il testo che desidera e poi, solo una volta premuto l'invio, il terminale elabora l'informazione. Inoltre in questa modalità i *tasti speciali* interrompono il programma e non hanno alcun significato a livello d byte.
- **Raw Mode**: Il terminale in questa modalità legge *for character*. Inoltre i *tasti speciali* qui possono venire gestiti in quanto vengono gestiti a livello di byte e non dal sistema.

Dunque quello che devo fare e creare dei metodi in grado di modificare la configurazione del terminale. Per fare questo vengono introdotte due librerie:
 - `<termios.h>`
 - `<unistd.h>`

