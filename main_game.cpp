#include "main_game.h"

#include "config_load.h"    // config file read

#include <cstdlib>          // rand()
#include <iostream>         // errors
#include <exception>        // terminate


mainGame::mainGame() :
    board(0, 0),
    form_size(0),
    total_weight(0),
    score(0),
    combo(0),
    max_score(0)
{
    for(size_t i=0; i<N_FORMS; i++) form[i] = (size_t)-1;
}

mainGame::mainGame(int _width, int _height) :
    board(_width, _height),
    form_size(0),
    total_weight(0),
    score(0),
    combo(0),
    max_score(0)
{
    for(size_t i=0; i<N_FORMS; i++) form[i] = (size_t)-1;
}



int mainGame::getform_size() const {
    return form_size;
}

int mainGame::getwidth() const {
    return board.getwidth();
}

int mainGame::getheight() const {
    return board.getheight();
}

int mainGame::getscore() const {
    return score;
}

int mainGame::getcombo() const {
    return combo;
}

int mainGame::getmax_score() const {
    return max_score;
}

void mainGame::setmax_score(int n) {
    max_score = n;
}


const int* mainGame::operator[](int n) const {
    return board[n];
}

mainGame::formEntry mainGame::getform(size_t n) const {
    return form[n] == (size_t) -1 ?
            formEntry{Form(), 0, COLOR_NONE} :
            form_set[form[n]];
}


bool mainGame::add_form(size_t n, int x, int y)
{
    // check validity
    if(form[n] == (size_t)-1) return false;
    // check that it can be added
    if(board.formCollide(form_set[form[n]].form, x, y)) return false;

    // add
    board.addForm(form_set[form[n]].form, x, y, form_set[form[n]].color);
    score += form_set[form[n]].form.getsize();
    // clean updating score
    int lines=0, columns=0;
    board.clean(lines, columns);
    update_score(lines, columns);

    if(score > max_score) max_score = score;

    // remove the added form
    form[n] = (size_t) -1;
    // select new ones if needed
    random_select_forms(false);

    return true;
}

bool mainGame::move_available() const
{
    for(size_t i=0; i<N_FORMS; i++)
    {
        if(form[i] != (size_t) -1)
        {
            Form f = form_set[form[i]].form;
            for(int x = - f.getboxmin().x;
                x < board.getwidth() - f.getboxmax().x; x++) {
                for(int y = - f.getboxmin().y;
                    y < board.getheight() - f.getboxmax().y; y++) {
                    if( !board.formCollide(f, x, y) ) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void mainGame::add_form_to_set(const Form &form, int color, unsigned int weight)
{
    if(form.getboxmax().x*2 +1 > form_size) form_size = form.getboxmax().x*2 +1;
    if(form.getboxmax().y*2 +1 > form_size) form_size = form.getboxmax().y*2 +1;
    if(-form.getboxmin().x*2 +1 > form_size) form_size = -form.getboxmin().x*2 +1;
    if(-form.getboxmin().y*2 +1 > form_size) form_size = -form.getboxmin().y*2 +1;

    form_set.push_back(mainGame::formEntry{form, weight, color});
    total_weight += weight;
}


void mainGame::random_select_forms(bool force)
{
    // test if forms must be selected
    if(!force) {
        bool empty = true;
        for(size_t i=0; i<N_FORMS && empty; i++) {
            if(form[i] != (size_t)-1) empty = false;
        }
        if(!empty) return;
    }

    if(form_set.size() == 0) {
        // empty set
        for(size_t i=0; i<N_FORMS; i++) {
            form[i] = (size_t) -1;
        }
    } else {
        for(size_t i=0; i<N_FORMS; i++) {
            /* the form array is parsed and each weight are substracted to n
             * the parsing stops when n<0, that weight, the probability to stop
             * on a particular form is proportional to its weight */
            int n = rand() % total_weight;
            size_t j = 0;
            while(n >= 0) {
                n -= form_set[j].weight;
                j++;
            }
            form[i] = j-1;
        }
    }
}

void mainGame::restart()
{
    board = Board(board.getwidth(), board.getheight());
    random_select_forms(true);
    score = 0; combo = 0;
}

void mainGame::reset()
{
    board = Board(board.getwidth(), board.getheight());
    form_set.clear();
    total_weight = 0;
    random_select_forms(true);
    score = 0; combo = 0;
}

void mainGame::update_score(int lines, int columns)
{
    score +=    lines*(lines+1)/2 * board.getwidth() +
                columns*(columns+1)/2 * board.getheight() +
                lines*columns * (board.getwidth() + board.getheight())/2;
    combo = lines + columns;
}



std::string mainGame::write() const
{
    std::string str;
    // header
    str += "WIDTH : " + std::to_string(board.getwidth()) + "\n";
    str += "HEIGHT : " + std::to_string(board.getheight()) + "\n";
    str += "\n\n";

    // form set
    for(size_t i=0; i<form_set.size(); i++) {
        str += "FORM : " + color_to_word(form_set[i].color) + "\n{\n";
        str += form_set[i].form.write();
        str += "}\n\n";
    }

    str += "\n";

    // save
    for(size_t i=0; i<N_FORMS; i++) {
        if(form[i] != (size_t)-1) {
            str += "SELECTED_FORM : " + std::to_string(i+1)
                    + " " + std::to_string(form[i]+1) + "\n";
        }
    }

    str += "\n\n";

    str += "BOARD :\n{\n" + board.write() + "}\n\n\n";
    str += "SCORE : " + std::to_string(score) + "\n";

    return str;
}


/* HERE BE DRAGONS */

mainGame mainGame::read(const std::string &str)
{
    std::string str_copy = str;     // work on a copy
    std::string line, key, value;
    mainGame game;

    // false until width and height are provided to initialize game
    bool initialized = false;

    int new_width=-1, new_height=-1;
    size_t n_line = 0, block_line = 0;  // line counter for errors
    size_t pos;                         // used for stoi()

    while(str_copy.size() > 0)
    {
        line = getline(str_copy);
        n_line++;
        clean_config_input(line);

        if(line.size() == 0) continue;

        if( !get_key_value(line, key, value) ) {
            syntax_exception excpt("invalid line in save file : " + line, n_line);
            throw excpt;
        } else if(key == "WIDTH") {
            if(new_width != -1) {
                syntax_exception excpt("illegal redefinition of width", n_line);
                throw excpt;
            }
            try {
                new_width = std::stoi(value, &pos);
            }
            catch(std::exception &e) {
                syntax_exception excpt("invalid input : " + line, n_line);
                throw excpt;
            }
            if(new_width < 0) {
                syntax_exception excpt("negative width value", n_line);
                throw excpt;
            }
            if(!blank_only(value.substr(pos))) {
                syntax_exception excpt("invalid input : " + line, n_line);
                throw excpt;
            }
        } else if(key == "HEIGHT") {
            if(new_height != -1) {
                syntax_exception excpt("illegal redefinition of height", n_line);
                throw excpt;
            }
            try {
                new_height = std::stoi(value, &pos);
            }
            catch(std::exception &e) {
                syntax_exception excpt("invalid input : " + line, n_line);
                throw excpt;
            }
            if(new_height < 0) {
                syntax_exception excpt("negative height value", n_line);
                throw excpt;
            }
            if(!blank_only(value.substr(pos))) {
                syntax_exception excpt("invalid input : " + line, n_line);
                throw excpt;
            }
        } else {
            // any other entry is illegal if game is not initialized
            if(!initialized) {
                syntax_exception excpt("width and height must be defined before any other input", n_line);
                throw excpt;
            }

            if(key == "FORM") {
                // extract color
                int color;
                int weight=1;
                try {
                    color = word_to_color(getword(value));
                }
                catch(syntax_exception &e) {
                    syntax_exception excpt(e.what(), n_line);
                    throw excpt;
                }
                if(color == COLOR_BLACK) {
                    syntax_exception excpt("forms can not be defined with color black", n_line);
                    throw excpt;
                }
                // extract weight (optional)
                if(!blank_only(value)) {
                    try {
                        weight = std::stoi(value, &pos);
                    }
                    catch(std::exception &e) {
                        syntax_exception excpt("invalid input : " + line, n_line);
                        throw excpt;
                    }
                    if(!blank_only(value.substr(pos))) {
                        syntax_exception excpt("invalid input : " + line, n_line);
                        throw excpt;
                    }
                    if(weight < 0) {
                        syntax_exception excpt("negative weight : " + line, n_line);
                        throw excpt;
                    }
                }
                // parse the block with form definition
                Form new_form;
                try {
                    new_form.read(getblock(str_copy, &block_line));
                }
                catch(syntax_exception &e) {
                    syntax_exception excpt(e.what(), n_line);
                    throw excpt;
                }
                game.add_form_to_set(new_form, color, weight);
                n_line += block_line;
            } else if(key == "BOARD") {
                try {
                    game.board.read(getblock(str_copy, &block_line));
                }
                catch(syntax_exception &e) {
                    syntax_exception excpt(e.what(), n_line);
                    throw excpt;
                }
                n_line += block_line;
            } else if(key == "SELECTED_FORM") {
                int index, form_index;

                std::string word = getword(value);
                try {
                    index = std::stoi(word, &pos) - 1;
                }
                catch(std::exception &e) {
                    syntax_exception excpt("invalid input : " + line, n_line);
                    throw excpt;
                }
                if(!blank_only(word.substr(pos))) {
                    syntax_exception excpt("invalid input : " + line, n_line);
                    throw excpt;
                }
                if(index < 0) {
                    syntax_exception excpt("negative index : " + line, n_line);
                    throw excpt;
                }
                try {
                    form_index = std::stoi(value, &pos)-1;
                }
                catch(std::exception &e) {
                    syntax_exception excpt("invalid input : " + line, n_line);
                    throw excpt;
                }
                if(!blank_only(value.substr(pos))) {
                    syntax_exception excpt("invalid input : " + line, n_line);
                    throw excpt;
                }
                if(form_index < 0) {
                    syntax_exception excpt("negative index : " + line, n_line);
                    throw excpt;
                }

                game.form[index] = form_index;
            } else if(key == "SCORE") {
                try {
                    game.score = std::stoi(value, &pos);
                }
                catch(std::exception &e) {
                    syntax_exception excpt("invalid input : " + line, n_line);
                    throw excpt;
                }
                if(!blank_only(value.substr(pos))) {
                    syntax_exception excpt("invalid input : " + line, n_line);
                    throw excpt;
                }
            } else {
                syntax_exception excpt("invalid key name : " + line, n_line);
                throw excpt;
            }
        }

        // create game if required info is given
        if(!initialized && new_width >= 0 && new_height >= 0) {
            game = mainGame(new_width, new_height);
            initialized = true;
        }
    }

    if(!initialized) {
        syntax_exception excpt("width and height must be defined");
        throw excpt;
    }

    return game;
}

/* END OF DRAGONS */


void mainGame::stream_write(std::ostream &os) const
{
    os << write();
}

mainGame mainGame::stream_read(std::istream &is)
{
    std::string input, line;
    while(std::getline(is, line))
    {
        input += line.substr(0, line.find('#')) + "\n";
    }
    return read(input);
}

