/*** Includes  ***/ 
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


/*** Defines  ***/ 
#define CTRL_KEY(k)     ((k) & 0x1f)  // 0x1f == 0b00011111


/*** Data  ***/ 
struct editorConfig {
    int screenRows;
    int screenCols;
    struct termios original_termios;
};

struct editorConfig editor;

/*** Terminal  ***/ 
void clearScreen() {
    // Clear the screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // Set cursor position
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void die(const char *s) {
    clearScreen();
    // Print a desciptive error message
    perror(s);
    exit(1);
}

void disableRawMode() {
    // Restore terminal attributes
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.original_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    // Get the current terminal attributes
    if (tcgetattr(STDIN_FILENO, &editor.original_termios) == -1)
        die("tcsetattr");

    // Function will be called automatically when the progam exits, whether it
    // exits by returning from main() or exit()
    atexit(disableRawMode);

    struct termios raw = editor.original_termios;

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

// It waits for one keypress and return it
char editorReadKey() {
    int nread;
    char c;

    // Read one character at a time from the user
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

int getWindowSize(int *cols, int *rows) {
    struct winsize windowSize;
    
    // Get window size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize) == -1 || windowSize.ws_col == 0) {
        return -1;
    } else {
        *cols = windowSize.ws_col;
        *rows = windowSize.ws_row;
        return 0;
    }
}


/*** Output  ***/ 
void editorDrawTildes() {
    // Display ~ on every row
    for (int y = 0; y < editor.screenRows; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    clearScreen();
    editorDrawTildes();

    // Set position of the cursor to the top (0 column, 0 row)
    write(STDOUT_FILENO, "\x1b[H", 3);
}


/*** Input  ***/ 
void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        // Quit application
        case CTRL_KEY('c'):
            clearScreen();
            exit(0);
            break;
    }
}


/*** Init  ***/ 
void initEditor() {
    // Check if we get window size
    if (getWindowSize(&editor.screenCols, &editor.screenRows) == -1)
        die("getWindowSize");
}

int main() {
    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}