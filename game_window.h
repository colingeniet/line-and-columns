#ifndef GAME_WINDOW_H_INCLUDED
#define GAME_WINDOW_H_INCLUDED

#include "main_game.h"  // used by gameWindow
#include "color.h"      // used for printing

#include <ncurses.h>    // used by gameWindow

#include <cstddef>      // size_t


/* main game display
 * Because copying ncurses windows does not make sense, this class
 * is not designed to be copied. */
class gameWindow
{
public:
    // input() return status
    enum returnValue
    {
        RETURN_NONE,
        RETURN_NO_MOVE,
        RETURN_QUIT,
        RETURN_MAX
    };

    gameWindow(mainGame*);
    ~gameWindow();

    void setgame(mainGame*);

    void print();

    returnValue input(int);


private:
    WINDOW *borderWindow, *boardWindow, *scoreWindow;
    WINDOW *formWindow[N_FORMS];
    mainGame *game;

    int cursor_x, cursor_y;
    size_t selected_form;

    // create all WINDOW objects (size calculation)
    void init_windows();

    // put cursor back inside bounds
    void cursor_bounds();

    // subwindows print functions
    void print_score();
    void print_board();
    void print_form(size_t);
};


#endif // GAME_WINDOW_H_INCLUDED
