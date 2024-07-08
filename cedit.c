#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

struct termios original_termios;

void disableRawMode () {
    // Restore terminal attributes
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enableRawMode() {
    // Get the current terminal attributes
    tcgetattr(STDIN_FILENO, &original_termios);

    // Function will be called automatically when the progam exits, whether it 
    // exits by returning from main() or exit()
    atexit(disableRawMode);

    struct termios raw = original_termios;

    // c_lflag (local flags) determines whether the terminal is operating 
    // in canonical or noncanonical mode
    // Disable echoing and cononical mode
    raw.c_lflag &= ~(ECHO | ICANON);

    // Set the modified terminal attributes 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {

    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return 0;
}