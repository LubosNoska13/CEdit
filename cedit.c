/*** Includes  ***/ 
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


/*** Data  ***/ 
struct termios original_termios;


/*** Terminal  ***/ 
void die(const char *s) {
    // Print a desciptive error message
    perror(s);
    exit(1);
}

void disableRawMode() {
    // Restore terminal attributes
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    // Get the current terminal attributes
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
        die("tcsetattr");

    // Function will be called automatically when the progam exits, whether it
    // exits by returning from main() or exit()
    atexit(disableRawMode);

    struct termios raw = original_termios;

    // ICRNL(Fix ctrl-m as 13), disable IXON (ctrl-s and ctrl-q)
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Turn off output processing
    raw.c_oflag &= ~(OPOST);

    raw.c_cflag |= (CS8);

    // Disable ECHO (echoing), ICANON (canonical mode), IEXTEN (ctrl-v),
    // ISIG (ctrl-c and ctrl-z)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // Minimum number of bytes nedded before read() can return 
    // We set it to 0, so read() returns as soon as there is any input
    raw.c_cc[VMIN] = 0;

    // Maximum amount of time to wait
    // 1/10 of second, ot 100 miliseconds
    raw.c_cc[VTIME] = 1;

    // Set the modified terminal attributes
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}


/*** Init  ***/ 
int main() {

    enableRawMode();

    while (1) {

        char c = '\0';
        // Read one character at a time from the user
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");

        // It tests whether a character is a control character
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }

        // Quit the application
        if (c == 'q') break;
    }

    return 0;
}