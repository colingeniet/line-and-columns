#include "menu_window.h"

#include <iostream>
#include <fstream>
#include <exception>


menuWindow::menuWindow(mainGame *newgame) :
    window(newwin(0,0,0,0)),
    game(newgame),
    selected_entry(0)
{
    entry[ENTRY_RESUME] = "resume";
    entry[ENTRY_RESTART] = "restart";
    entry[ENTRY_SAVE] = "save game";
    entry[ENTRY_LOAD] = "load game";
    entry[ENTRY_LAST_SAVE] = "load last game";
    entry[ENTRY_DEFAULT_SETTING] = "reset game rules";
    entry[ENTRY_SCORES] = "highscores";
    entry[ENTRY_QUIT] = "quit";
}

menuWindow::~menuWindow()
{
    delwin(window);
}

void menuWindow::setgame(mainGame *newgame)
{
    game = newgame;
}


void menuWindow::print()
{
    wclear(window);

    int maxx, maxy, x, y;
    getmaxyx(window, maxy, maxx);

    y = (maxy - ENTRY_MAX + 1)/2;

    wattrset(window, A_BOLD);

    for(size_t i=0; i<ENTRY_MAX; i++) {
        x = (maxx - entry[i].size() + 1)/2;
        if(i == selected_entry) wattron(window, A_REVERSE);
        mvwprintw(window, y+i, x, "%s", entry[i].c_str());
        if(i == selected_entry) wattroff(window, A_REVERSE);
    }

    wnoutrefresh(window);
}

menuWindow::returnValue menuWindow::input(int ch)
{
    MEVENT event;

    switch(ch)
    {
    case KEY_UP:
        if(selected_entry == 0) selected_entry = ENTRY_MAX-1;
        else selected_entry--;
        break;
    case KEY_DOWN:
        if(selected_entry == ENTRY_MAX-1) selected_entry = 0;
        else selected_entry++;
        break;
    case '\n':
    case '\r':
    case KEY_ENTER:
        return excecute_entry(selected_entry);
        break;
    case KEY_MOUSE:
        if(getmouse(&event) == OK)
        {
            if(wenclose(window, event.y, event.x))
            {
                wmouse_trafo(window, &event.y, &event.x, false);
                int x, y, maxx, maxy;
                getmaxyx(window, maxy, maxx);
                y = (maxy - ENTRY_MAX + 1)/2;
                for(size_t i=0; i<ENTRY_MAX; i++) {
                    x = (maxx - entry[i].size() + 1)/2;
                    if(event.y == y+(int)i &&
                       x <= event.x && event.x < x + (int)entry[i].size())
                    {
                        selected_entry = i;
                        if(event.bstate & BUTTON1_PRESSED) {
                            return excecute_entry(i);
                        }
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    return RETURN_NONE;
}



menuWindow::returnValue menuWindow::excecute_entry(int entry)
{
    selected_entry = 0;
    switch(entry)
    {
    case ENTRY_RESUME:
        return RETURN_RESUME;
        break;
    case ENTRY_RESTART:
        game->restart();
        return RETURN_RESUME;
        break;
    case ENTRY_SAVE:
        save( prompt("Save as :").c_str(), true);
        return RETURN_NONE;
        break;
    case ENTRY_LOAD:
        if(load( prompt("Load file :").c_str(), true ))
            return RETURN_UPDATE_GAME;
        else
            return RETURN_NONE;
        break;
    case ENTRY_LAST_SAVE:
        if(load(AUTOSAVE_FILE, false))
            return RETURN_UPDATE_GAME;
        else
            return RETURN_NONE;
        break;
    case ENTRY_DEFAULT_SETTING:
        if(load(DEFAULT_BOARD, false))
            return RETURN_UPDATE_GAME;
        else
            return RETURN_NONE;
        break;
    case ENTRY_SCORES:
        return RETURN_SCORES;
        break;
    case ENTRY_QUIT:
        return RETURN_QUIT;
        break;
    default:
        std::cerr << "Incorrect menu code" << std::endl;
        std::terminate();
        break;
    }
}


std::string menuWindow::prompt(const std::string &prompt) const
{
    std::string name;
    int maxx, x;
    getmaxyx(window, x, maxx);

    // reactivate cursor visibility
    curs_set(1);

    for(;;) {
        wclear(window);
        x = maxx/2 - prompt.size();
        mvwprintw(window, 1, x, "%s %s", prompt.c_str(), name.c_str());
        wnoutrefresh(window);
        doupdate();

        int ch = getch();
        if(ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            if(name.size() > 0) break;
        } else if(ch == '\b' || ch == 127 ||
                  ch == KEY_BACKSPACE || ch == KEY_DC) {
            if(name.size() > 0) name.pop_back();
        } else {
            name += ch;
        }
    }

    curs_set(0);

    return name;
}

void menuWindow::save(const char *file, bool verbose) const
{
    std::string success_msg = "Save successfull - press any key";
    std::string error_msg = "Save failed - press any key";
    int maxx, maxy;
    getmaxyx(window, maxy, maxx);

    std::ofstream output(file);
    if(!output.is_open()) {
        if(verbose)
        {
            wclear(window);
            mvwprintw(window, maxy/2, (maxx-error_msg.size())/2,
                      "%s", error_msg.c_str());
            wnoutrefresh(window);
            doupdate();
        }
    } else {
        game->stream_write(output);
        if(verbose)
        {
            wclear(window);
            mvwprintw(window, maxy/2, (maxx-success_msg.size())/2,
                      "%s", success_msg.c_str());
            wnoutrefresh(window);
            doupdate();
        }
    }
    output.close();

    if(verbose)
    {
        int ch;
        do {
            ch = getch();
        } while(ch == KEY_MOUSE);
    }

}

bool menuWindow::load(const char *file, bool verbose)
{
    std::string success_msg = "Save successfully loaded - press any key";
    std::string error_open_msg = "Failed to open save - press any key";
    std::string error_read_msg = "Save invalid - press any key";
    int maxx, maxy;
    getmaxyx(window, maxy, maxx);

    std::ifstream input(file);
    if(!input.is_open()) {
        if(verbose)
        {
            wclear(window);
            mvwprintw(window, maxy/2, (maxx-error_open_msg.size())/2,
                      "%s", error_open_msg.c_str());
            wnoutrefresh(window);
            doupdate();
        }
    } else {
        mainGame tmp;
        try {
            tmp = mainGame::stream_read(input);
            *game = tmp;

            input.close();

            if(verbose)
            {
                wclear(window);
                mvwprintw(window, maxy/2, (maxx-success_msg.size())/2,
                          "%s", success_msg.c_str());
                wnoutrefresh(window);
                doupdate();

                int ch;
                do {
                    ch = getch();
                } while(ch == KEY_MOUSE);
            }

            return true;
        }
        catch(std::exception &excpt) {
            input.close();

            if(verbose)
            {
               std::string error(excpt.what());
                wclear(window);
                mvwprintw(window, maxy/2, (maxx-error.size())/2,
                          "%s", error.c_str());
                mvwprintw(window, maxy/2 + 1, (maxx-error_read_msg.size())/2,
                          "%s", error_read_msg.c_str());
                wnoutrefresh(window);
                doupdate();
            }
        }
    }

    if(verbose)
    {
        int ch;
        do {
            ch = getch();
        } while(ch == KEY_MOUSE);
    }

    return false;
}
