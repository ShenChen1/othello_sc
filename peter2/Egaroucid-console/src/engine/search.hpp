/*
    Egaroucid Project

    @file search.hpp
        Search common structure
    @date 2021-2023
    @author Takuto Yamana
    @license GPL-3.0 license
    @notice I referred to codes written by others
*/

#pragma once
#include <iostream>
#include <fstream>
#include <algorithm>
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "thread_pool.hpp"
#include "evaluate.hpp"

/*
    @brief Evaluation constant
*/
#define N_SYMMETRY_PATTERNS 62
#if USE_SIMD_EVALUATION
    #define N_SIMD_EVAL_FEATURES 4
#endif

/*
    @brief Search switch parameters
*/
#define MID_FAST_DEPTH 1
#define MID_FAST_NWS_DEPTH 1
#define END_FAST_DEPTH 6
#define END_SIMPLE_DEPTH 10
#define MID_TO_END_DEPTH 13

/*
    @brief Search hyperparameters
*/
#define FAIL_HIGH_WISH_THRESHOLD_END_NWS 4
#define MID_ETC_DEPTH 14



/*
    @brief Search constant
*/
#define SCORE_UNDEFINED -SCORE_INF
#ifndef SEARCH_BOOK
    #define SEARCH_BOOK -1
#endif


/*
    @brief Weights of each cell
*/
constexpr int cell_weight[HW2] = {
    18,  4,  16, 12, 12, 16,  4, 18,
     4,  2,   6,  8,  8,  6,  2,  4,
    16,  6,  14, 10, 10, 14,  6, 16,
    12,  8,  10,  0,  0, 10,  8, 12,
    12,  8,  10,  0,  0, 10,  8, 12,
    16,  6,  14, 10, 10, 14,  6, 16,
     4,  2,   6,  8,  8,  6,  2,  4,
    18,  4,  16, 12, 12, 16,  4, 18
};

/*
    @brief Stability cutoff threshold
    from https://github.com/abulmo/edax-reversi/blob/1ae7c9fe5322ac01975f1b3196e788b0d25c1e10/src/search.c#L108 and modified
*/
constexpr int stability_threshold_nws[HW2] = {
    99, 99, 99, 99, 99, 99, 99, 99, 
    99, 99, 99, 99, 99, 99, 99, 99, 
    99, 64, 64, 64, 64, 64, 64, 64, 
    64, 62, 62, 60, 60, 58, 58, 56, 
    56, 54, 54, 52, 52, 50, 50, 48, 
    48, 46, 44, 42, 40, 38, 36, 34, 
    32, 30, 28, 26, 24, 22, 20, 16, 
    14, 12, 10, 8, 6, 99, 99, 99
};

constexpr int stability_threshold[HW2] = {
    99, 99, 99, 99, 99, 99, 99, 99, 
    99, 99, 99, 99, 99, 99, 99, 99, 
    99, 62, 62, 60, 60, 58, 58, 56, 
    56, 54, 54, 52, 52, 50, 50, 48, 
    48, 46, 46, 44, 44, 42, 42, 40, 
    40, 38, 36, 34, 32, 30, 28, 26, 
    24, 22, 20, 18, 16, 14, 12, 8, 
    6, 4, 2, 0, -2, 99, 99, 99
};

/*
    @brief board division

    used for parity calculation
*/
constexpr uint_fast8_t cell_div4[HW2] = {
    1, 1, 1, 1, 2, 2, 2, 2, 
    1, 1, 1, 1, 2, 2, 2, 2, 
    1, 1, 1, 1, 2, 2, 2, 2, 
    1, 1, 1, 1, 2, 2, 2, 2, 
    4, 4, 4, 4, 8, 8, 8, 8, 
    4, 4, 4, 4, 8, 8, 8, 8, 
    4, 4, 4, 4, 8, 8, 8, 8, 
    4, 4, 4, 4, 8, 8, 8, 8
};

/*
    @brief board division

    used for parity calculation
*/
constexpr uint_fast8_t cell_div4_log[HW2] = {
    0, 0, 0, 0, 1, 1, 1, 1, 
    0, 0, 0, 0, 1, 1, 1, 1, 
    0, 0, 0, 0, 1, 1, 1, 1, 
    0, 0, 0, 0, 1, 1, 1, 1, 
    2, 2, 2, 2, 3, 3, 3, 3, 
    2, 2, 2, 2, 3, 3, 3, 3, 
    2, 2, 2, 2, 3, 3, 3, 3, 
    2, 2, 2, 2, 3, 3, 3, 3
};

/*
    @brief a table for parity-based move ordering
*/
constexpr uint64_t parity_table[16] = {
    0x0000000000000000ULL, 0x000000000F0F0F0FULL, 0x00000000F0F0F0F0ULL, 0x00000000FFFFFFFFULL,
    0x0F0F0F0F00000000ULL, 0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0FF0F0F0F0ULL, 0x0F0F0F0FFFFFFFFFULL,
    0xF0F0F0F000000000ULL, 0xF0F0F0F00F0F0F0FULL, 0xF0F0F0F0F0F0F0F0ULL, 0xF0F0F0F0FFFFFFFFULL,
    0xFFFFFFFF00000000ULL, 0xFFFFFFFF0F0F0F0FULL, 0xFFFFFFFFF0F0F0F0ULL, 0xFFFFFFFFFFFFFFFFULL
};

/*
    @brief Search result structure

    Used for returning the result

    @param policy               selected move
    @param value                value
    @param depth                search depth
    @param time                 elapsed time
    @param nodes                number of nodes visited
    @param clog_time            elapsed time for clog search
    @param clog_nodes           number of nodes visited for clog search
    @param nps                  NPS (Nodes Per Second)
    @param is_end_search        search till the end?
    @param probability          MPC (Multi-ProbCut) probability in integer [%]
*/
struct Search_result{
    int_fast8_t policy;
    int value;
    int depth;
    uint64_t time;
    uint64_t nodes;
    uint64_t clog_time;
    uint64_t clog_nodes;
    uint64_t nps;
    bool is_end_search;
    int probability;

    Search_result(){
        policy = HW2;
        value = SCORE_UNDEFINED;
        depth = -1;
        time = 0;
        nodes = 0;
        clog_time = 0;
        clog_nodes = 0;
        nps = 0;
        is_end_search = false;
        probability = 0;
    }

    bool operator<(const Search_result &another) const{
        if (depth == SEARCH_BOOK && another.depth != SEARCH_BOOK)
            return false;
        else if (depth != SEARCH_BOOK && another.depth == SEARCH_BOOK)
            return true;
        return value < another.value;
    }

    bool operator>(const Search_result &another) const{
        if (another.depth == SEARCH_BOOK && depth != SEARCH_BOOK)
            return false;
        else if (another.depth != SEARCH_BOOK && depth == SEARCH_BOOK)
            return true;
        return value > another.value;
    }
};

/*
    @brief Analyze result structure

    Used in `analyze` command

    @param played_move          played move
    @param played_score         score of played move
    @param played_depth         depth of search for played_score calculation
    @param played_probability   probability of search for played_score calculation
    @param alt_move             alternative best move
    @param alt_score            score of alternative move
    @param alt_depth            depth of search for alt_score calculation
    @param alt_probability      probability of search for alt_score calculation
*/
struct Analyze_result{
    int played_move;
    int played_score;
    int played_depth;
    int played_probability;
    int alt_move;
    int alt_score;
    int alt_depth;
    int alt_probability;
};

#if USE_SIMD_EVALUATION
    union Eval_features{
        __m256i f256[N_SIMD_EVAL_FEATURES];
        __m128i f128[N_SIMD_EVAL_FEATURES * 2];
    };
#endif

/*
    @brief Search structure

    Used in midgame / endgame search

    @param board                board to solve
    @param n_discs              number of discs on the board
    @param parity               parity of the board
    @param mpc_level            MPC (Multi-ProbCut) probability level
    @param n_nodes              number of visited nodes
    @param eval_features        features of pattern evaluation
    @param eval_feature_reversed    need to swap player in evaluation?
    @param use_multi_thread     use parallel search?
*/
class Search{
    public:
        Board board;
        int_fast8_t strt_n_discs;
        int_fast8_t n_discs;
        uint_fast8_t parity;
        uint_fast8_t mpc_level;
        uint64_t n_nodes;
        #if USE_SIMD_EVALUATION
            Eval_features eval_features[HW2 - 4];
            uint_fast8_t eval_feature_idx;
        #else
            uint_fast16_t eval_features[N_SYMMETRY_PATTERNS];
        #endif
        bool eval_feature_reversed;
        bool use_multi_thread;
        #if USE_SEARCH_STATISTICS
            uint64_t n_nodes_discs[HW2];
        #endif

    public:
        /*
            @brief Initialize with board

            @param init_board           a board to set
        */
        inline void init_board(Board *init_board){
            board = init_board->copy();
            n_discs = board.n_discs();
            strt_n_discs = n_discs;
            uint64_t empty = ~(board.player | board.opponent);
            parity = 1 & pop_count_ull(empty & 0x000000000F0F0F0FULL);
            parity |= (1 & pop_count_ull(empty & 0x00000000F0F0F0F0ULL)) << 1;
            parity |= (1 & pop_count_ull(empty & 0x0F0F0F0F00000000ULL)) << 2;
            parity |= (1 & pop_count_ull(empty & 0xF0F0F0F000000000ULL)) << 3;
        }

        /*
            @brief Initialize Search menber variables
        */
        inline void init_search(){
            n_discs = board.n_discs();
            strt_n_discs = n_discs;
            uint64_t empty = ~(board.player | board.opponent);
            parity = 1 & pop_count_ull(empty & 0x000000000F0F0F0FULL);
            parity |= (1 & pop_count_ull(empty & 0x00000000F0F0F0F0ULL)) << 1;
            parity |= (1 & pop_count_ull(empty & 0x0F0F0F0F00000000ULL)) << 2;
            parity |= (1 & pop_count_ull(empty & 0xF0F0F0F000000000ULL)) << 3;
        }

        /*
            @brief Move board and other variables except eval_features

            @param flip                 Flip information
        */
        inline void move(const Flip *flip) {
            board.move_board(flip);
            ++n_discs;
            parity ^= cell_div4[flip->pos];
        }

        /*
            @brief Undo board and other variables except eval_features

            @param flip                 Flip information
        */
        inline void undo(const Flip *flip) {
            board.undo_board(flip);
            --n_discs;
            parity ^= cell_div4[flip->pos];
        }

        /*
            @brief Move board and other variables except eval_features

            @param flip                 Flip information
        */
        inline void move_lastN(const Flip *flip) {
            board.move_board(flip);
            #if !USE_SIMD
                parity ^= cell_div4[flip->pos];
            #endif
        }

        /*
            @brief Undo board and other variables except eval_features

            @param flip                 Flip information
        */
        inline void undo_lastN(const Flip *flip) {
            board.undo_board(flip);
            #if !USE_SIMD
                parity ^= cell_div4[flip->pos];
            #endif
        }

        /*
            @brief Get evaluation phase
        */
        inline int phase(){
            return (n_discs - 4) / PHASE_N_STONES;
            //return std::min(N_PHASES - 1, (n_discs - 4) / PHASE_N_STONES);
        }
};

/*
    @brief Clog search structure

    @param board                board to solve
    @param n_nodes              number of visited nodes
*/
struct Clog_search{
    Board board;
    uint64_t n_nodes;
};

/*
    @brief Clog search result structure

    @param pos                  position to put
    @param val                  the exact score
*/
struct Clog_result{
    uint_fast8_t pos;
    int val;
};