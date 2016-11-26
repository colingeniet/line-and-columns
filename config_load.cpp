#include "config_load.h"
#include "color.h"

#include <iostream>
#include <exception>



bool blank_only(const std::string &str)
{
    return str.find_first_not_of(" \t\n") == std::string::npos;
}


void clean_config_input(std::string &input)
{
    // begining / end blanks
    size_t start, end;
    start = input.find_first_not_of(" \n\t");
    end = input.find_last_not_of(" \n\t");
    if(start == std::string::npos) {
        input = std::string();
        return;
    } else {
        input = input.substr(start, end-start+1);
    }

    // cleaning
    start = end = 0;
    start = input.find_first_of(" \n\t", start+1);
    while(start != std::string::npos)
    {
        end = input.find_first_not_of(" \n\t", start);
        input = input.substr(0, start) + " "
                + input.substr(end, std::string::npos);
        start = input.find_first_of(" \n\t", start+1);
    }
}


std::string getword(std::string &input)
{
    size_t blank = input.find_first_of(" \t\n");
    std::string word = input.substr(0, blank);

    if(blank == std::string::npos)
        input = std::string();
    else
        input = input.substr(blank+1, std::string::npos);

    return word;
}

std::string getline(std::string &input)
{
    size_t blank = input.find_first_of("\n");
    std::string line = input.substr(0, blank);

    if(blank == std::string::npos)
        input = std::string();
    else
        input = input.substr(blank+1, std::string::npos);

    return line;
}

std::string getblock(std::string &input)
{
    std::string block;

    // find delimiters
    size_t start = input.find('{');
    if(start == std::string::npos) {
        std::cerr << "Invalid block syntax : missing {" << std::endl;
        std::terminate();
        return std::string();
    }
    size_t end = input.find('}', start);
    if(start == std::string::npos) {
        std::cerr << "Invalid block syntax : missing }" << std::endl;
        std::terminate();
        return std::string();
    }

    // check for blank only before {
    block = input.substr(0, start);
    if(!blank_only(block)) {
        std::cerr << "Invalid block syntax : input before {" << std::endl;
        std::terminate();
        return std::string();
    }

    block = input.substr(start+1, end-start-1);
    input = input.substr(end+1);
    return block;
}


bool get_key_value(const std::string &input,
                   std::string &key, std::string &value)
{
    size_t delimiter = input.find(" :");

    // special case : at the end of input, second space is optional
    if(delimiter == input.size()-2) {
        key = input.substr(0, delimiter);
        value = std::string();
        return true;
    } else {
        delimiter = input.find(" : ");

        if(delimiter == std::string::npos) return false;

        key = input.substr(0, delimiter);
        value = input.substr(delimiter+3, std::string::npos);
        return true;
    }
}


void set_case(std::string &str, bool lower)
{
    if(lower)
    {
        for(size_t i=0; i<str.size(); i++) {
            if('A' <= str[i] && str[i] <= 'Z')
                str[i] += 'a' - 'A';
        }
    }
    else
    {
        for(size_t i=0; i<str.size(); i++) {
            if('a' <= str[i] && str[i] <= 'z')
                str[i] += 'A' - 'a';
        }
    }
}



int word_to_color(const std::string& input)
{
    std::string word = input;
    if(word == "BLACK")         return COLOR_BLACK;
    else if(word == "RED")      return COLOR_RED;
    else if(word == "GREEN")    return COLOR_GREEN;
    else if(word == "YELLOW")   return COLOR_YELLOW;
    else if(word == "BLUE")     return COLOR_BLUE;
    else if(word == "MAGENTA")  return COLOR_MAGENTA;
    else if(word == "CYAN")     return COLOR_CYAN;
    else if(word == "WHITE")    return COLOR_WHITE;
    else {
        std::cerr << "Invalid color name : " << input << std::endl;
        return -1;
    }
}


std::string color_to_word(int color)
{
    switch(color)
    {
    case COLOR_BLACK:
        return "BLACK";
        break;
    case COLOR_RED:
        return "RED";
        break;
    case COLOR_GREEN:
        return "GREEN";
        break;
    case COLOR_YELLOW:
        return "YELLOW";
        break;
    case COLOR_BLUE:
        return "BLUE";
        break;
    case COLOR_MAGENTA:
        return "MAGENTA";
        break;
    case COLOR_CYAN:
        return "CYAN";
        break;
    case COLOR_WHITE:
        return "WHITE";
        break;
    default:
        std::cerr << "Invalid color code : " << color << std::endl;
        return "";
        break;
    }
}
