/*
    Egaroucid Project

    @file commandline_option_definition.hpp
        Definition of commandline options
    @date 2021-2023
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <string>
#include <vector>

#define N_COMMANDLINE_OPTIONS 15

#define ID_VERSION 0
#define ID_HELP 1
#define ID_LEVEL 2
#define ID_THREAD 3
#define ID_LOG 4
#define ID_HASH 5
#define ID_LEVEL_INFO 6
#define ID_BOOK_FILE 7
#define ID_EVAL_FILE 8
#define ID_NOBOOK 9
#define ID_SOLVE 10
#define ID_MODE 11
#define ID_GTP 12
#define ID_QUIET 13
#define ID_SELF_PLAY 14

struct Commandline_option_info{
    int id;
    std::vector<std::string> names;
    std::string arg;
    std::string description;
};

const Commandline_option_info commandline_option_data[N_COMMANDLINE_OPTIONS] = {
    {ID_VERSION,    {"-v", "-version"},                                 "",                 "Check Egaroucid for Console version"}, 
    {ID_HELP,       {"-h", "-help", "-?"},                              "",                 "See help"}, 
    {ID_LEVEL,      {"-l", "-level"},                                   "<level>",          "Set level to <level> (0 to 60)"}, 
    {ID_THREAD,     {"-t", "-thread", "-threads"},                      "<n_threads>",      "Set number of threads (more than 0)"},
    {ID_LOG,        {"-noise"},                                         "",                 "Show all logs"},
    {ID_HASH,       {"-hash", "-hashlevel"},                            "<hash_level>",     "Set hash level to <hash_level> (0 to 29)"},
    {ID_LEVEL_INFO, {"-linfo", "-levelinfo"},                           "",                 "See level information"},
    {ID_BOOK_FILE,  {"-b", "-book"},                                    "<book_file>",      "Import <book_file> as Egaroucid's book"},
    {ID_EVAL_FILE,  {"-eval", "-evaluation"},                           "<eval_file>",      "Import <eval_file> as Egaroucid's evaluation function"},
    {ID_NOBOOK,     {"-nobook"},                                        "",                 "Run Egaroucid without book"},
    {ID_SOLVE,      {"-s", "-solve", "-sol"},                           "<problem file>",   "Solve problems written in <problem file>"},
    {ID_MODE,       {"-m", "-mode"},                                    "<mode>",           "Set mode to <mode> (0: You vs Egaroucid, 1: Egaroucid vs You, 2: Egaroucid vs Egaroucid, 3: You vs You)"},
    {ID_GTP,        {"-gtp"},                                           "",                 "Use GTP (Go Text Protocol) mode"},
    {ID_QUIET,      {"-q", "-quiet", "-silent"},                        "",                 "Quiet mode"},
    {ID_SELF_PLAY,  {"-selfplay"},                                      "<n>",              "Self play n games (play randomly first m moves, where 10 <= m <= 19)"}
};