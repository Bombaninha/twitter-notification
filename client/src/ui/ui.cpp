#include "ui.hpp"

#include <ncurses.h>
#include <string>

using namespace std;

WINDOW* create_feed() {
    int termWidth, termHeight;
    getmaxyx(stdscr, termHeight, termWidth);

    WINDOW* feed = newwin(termHeight - 3, termWidth - 30, 0, 0);
    refresh();

    box(feed, 0, 0);
    mvwprintw(feed, 0, 1, "Feed");
    wrefresh(feed);

    return feed;
}

WINDOW* create_input() {
    int termWidth, termHeight;
    getmaxyx(stdscr, termHeight, termWidth);

    WINDOW* input = newwin(3, termWidth - 30, termHeight - 3, 0);
    refresh();

    box(input, 0, 0);
    mvwprintw(input, 0, 1, "Commands");
    mvwprintw(input, 1, 1, ">");
    wrefresh(input);

    return input;
}

WINDOW* create_help(string username) {
    int termWidth, termHeight;
    getmaxyx(stdscr, termHeight, termWidth);

    WINDOW* help = newwin(termHeight, 30, 0, termWidth - 30);
    refresh();

    box(help, 0, 0);
    mvwprintw(help, 0, 1, "Help");

    mvwprintw(help, 1, 1, "Logged in as: %s", username.c_str());

    mvwprintw(help, 3, 1, "Commands:");
    mvwprintw(help, 4, 3, "FOLLOW @username");
    mvwprintw(help, 5, 3, "SEND text to send");
    mvwprintw(help, 6, 3, "EXIT to exit");

    wrefresh(help);

    return help;
}

void initUI(WINDOW** input, WINDOW** output, WINDOW** help, string username) {
    initscr();
    cbreak();
    noecho();

    *input = create_input();
    *output = create_feed();
    *help = create_help(username);
}

void destroyUI(WINDOW* input, WINDOW* output, WINDOW* help) {
    delwin(input);
    delwin(output);
    delwin(help);

    endwin();
}

void clearInput(string* userInput, WINDOW* input) {
    string whiteString = string(userInput->length(), ' ');
    
    userInput->clear();

    mvwprintw(input, 1, 3, whiteString.c_str());
    mvwprintw(input, 1, 3, "");
    wrefresh(input);
}

int kbhit(void) {
    int ch, r;

    nodelay(stdscr, TRUE);

    ch = getch();

    if (ch == ERR) {
        r = FALSE;
    } else {
        r = TRUE;
        ungetch(ch);
    }

    nodelay(stdscr, FALSE);
    
    return r;
}