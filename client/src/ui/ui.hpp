#if !defined(UI_H)
#define UI_H

#include <ncurses.h>
#include <string>

void initUI(WINDOW** input, WINDOW** output, WINDOW** help);
void destroyUI(WINDOW* input, WINDOW* output, WINDOW* help);
void clearInput(std::string* userInput, WINDOW* input);
int kbhit();

#endif // UI_H
