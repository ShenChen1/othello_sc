/*
    Egaroucid Project

    @file midsearch_nws.hpp
        Search midgame with NWS (Null Window Search)
    @date 2021-2023
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <future>
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
#include "endsearch.hpp"
#include "move_ordering.hpp"
#include "probcut.hpp"
#include "thread_pool.hpp"
#include "ybwc.hpp"
#include "util.hpp"
#include "stability.hpp"

inline bool ybwc_split_nws(const Search *search, int alpha, int depth, uint64_t legal, bool is_end_search, const bool *searching, uint_fast8_t policy, const int pv_idx, const int move_idx, const int canput, const int running_count, std::vector<std::future<Parallel_task>> &parallel_tasks);
inline void ybwc_get_end_tasks(Search *search, std::vector<std::future<Parallel_task>> &parallel_tasks, int *v, int *best_move, int *running_count);
inline void ybwc_wait_all_nws(Search *search, std::vector<std::future<Parallel_task>> &parallel_tasks, int *v, int *best_move, int *running_count, int alpha, const bool *searching, bool *n_searching);

/*
    @brief Get a value with last move with Nega-Alpha algorithm (NWS)

    No move ordering. Just search it.

    @param search               search information
    @param alpha                alpha value (beta value is alpha + 1)
    @param skipped              already passed?
    @param searching            flag for terminating this search
    @return the value
*/
inline int nega_alpha_eval1_nws(Search *search, int alpha, bool skipped, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    ++search->n_nodes;
    #if USE_SEARCH_STATISTICS
        ++search->n_nodes_discs[search->n_discs];
    #endif
    int v = -SCORE_INF;
    uint64_t legal = search->board.get_legal();
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_eval1_nws(search, -alpha - 1, true, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    int g;
    Flip flip;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &search->board, cell);
        eval_move(search, &flip);
        search->move(&flip);
            g = -mid_evaluate_diff(search);
        search->undo(&flip);
        eval_undo(search, &flip);
        ++search->n_nodes;
        if (v < g){
            if (alpha < g){
                return g;
            }
            v = g;
        }
    }
    return v;
}

#if MID_FAST_NWS_DEPTH > 1
    /*
        @brief Get a value with last few moves with Nega-Alpha algorithm (NWS)

        No move ordering. Just search it.

        @param search               search information
        @param alpha                alpha value (beta value is alpha + 1)
        @param depth                remaining depth
        @param skipped              already passed?
        @param searching            flag for terminating this search
        @return the value
    */
    int nega_alpha_nws(Search *search, int alpha, int depth, bool skipped, const bool *searching){
        if (!global_searching || !(*searching))
            return SCORE_UNDEFINED;
        ++search->n_nodes;
        #if USE_SEARCH_STATISTICS
            ++search->n_nodes_discs[search->n_discs];
        #endif
        if (depth == 1)
            return nega_alpha_eval1_nws(search, alpha, skipped, searching);
        if (depth == 0)
            return mid_evaluate_diff(search);
        uint64_t legal = search->board.get_legal();
        int v = -SCORE_INF;
        if (legal == 0ULL){
            if (skipped)
                return end_evaluate(&search->board);
            search->eval_feature_reversed ^= 1;
            search->board.pass();
                v = -nega_alpha_nws(search, -alpha - 1, depth, true, searching);
            search->board.pass();
            search->eval_feature_reversed ^= 1;
            return v;
        }
        uint32_t hash_code = search->board.hash();
        int lower = -SCORE_MAX, upper = SCORE_MAX;
        uint_fast8_t moves[N_TRANSPOSITION_MOVES] = {TRANSPOSITION_TABLE_UNDEFINED, TRANSPOSITION_TABLE_UNDEFINED};
        #if MID_TO_END_DEPTH < USE_TT_DEPTH_THRESHOLD
            if (search->n_discs <= HW2 - USE_TT_DEPTH_THRESHOLD)
                transposition_table.get(search, hash_code, depth, &lower, &upper, moves);
        #else
            transposition_table.get(search, hash_code, depth, &lower, &upper, moves);
        #endif
        if (upper == lower)
            return upper;
        if (alpha < lower)
            return lower;
        if (upper <= alpha)
            return upper;
        #if USE_MID_MPC
            #if 1 + 1 < USE_MPC_DEPTH
                if (depth >= USE_MPC_DEPTH){
                    if (mpc(search, alpha, alpha + 1, depth, legal, false, &v, searching))
                        return v;
                }
            #else
                if (mpc(search, alpha, alpha + 1, depth, legal, false, &v, searching))
                    return v;
            #endif
        #endif
        int best_move = TRANSPOSITION_TABLE_UNDEFINED;
        int g;
        const int canput = pop_count_ull(legal);
        std::vector<Flip_value> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
            calc_flip(&move_list[idx].flip, &search->board, cell);
            if (move_list[idx].flip.flip == search->board.opponent)
                return SCORE_MAX;
            ++idx;
        }
        move_list_evaluate_nws_fast(search, move_list, moves, depth, alpha, searching);
        for (int move_idx = 0; move_idx < canput && *searching; ++move_idx){
            swap_next_best_move(move_list, move_idx, canput);
            eval_move(search, &move_list[move_idx].flip);
            search->move(&move_list[move_idx].flip);
                g = -nega_alpha_nws(search, -alpha - 1, depth - 1, false, searching);
            search->undo(&move_list[move_idx].flip);
            eval_undo(search, &move_list[move_idx].flip);
            if (v < g){
                v = g;
                best_move = move_list[move_idx].flip.pos;
                if (alpha < v)
                    break;
            }
        }
        if (*searching && global_searching)
            transposition_table.reg(search, hash_code, depth, alpha, alpha + 1, v, best_move);
        return v;
    }
#endif

/*
    @brief Get a value with given depth with Nega-Alpha algorithm (NWS)

    Search with move ordering for midgame NWS
    Parallel search (YBWC: Young Brothers Wait Concept) used.

    @param search               search information
    @param alpha                alpha value (beta value is alpha + 1)
    @param depth                remaining depth
    @param skipped              already passed?
    @param legal                for use of previously calculated legal bitboard
    @param is_end_search        search till the end?
    @param searching            flag for terminating this search
    @return the value
*/
int nega_alpha_ordering_nws(Search *search, int alpha, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (is_end_search && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end_nws(search, alpha, skipped, legal, false, searching);
    if (!is_end_search){
        #if MID_FAST_NWS_DEPTH > 1
            if (depth <= MID_FAST_NWS_DEPTH)
                return nega_alpha_nws(search, alpha, depth, skipped, searching);
        #else
            if (depth == 1)
                return nega_alpha_eval1_nws(search, alpha, skipped, searching);
            if (depth == 0)
                return mid_evaluate_diff(search);
        #endif
    }
    ++search->n_nodes;
    #if USE_SEARCH_STATISTICS
        ++search->n_nodes_discs[search->n_discs];
    #endif
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int v = -SCORE_INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_ordering_nws(search, -alpha - 1, depth, true, LEGAL_UNDEFINED, is_end_search, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    uint32_t hash_code = search->board.hash();
    int lower = -SCORE_MAX, upper = SCORE_MAX;
    uint_fast8_t moves[N_TRANSPOSITION_MOVES] = {TRANSPOSITION_TABLE_UNDEFINED, TRANSPOSITION_TABLE_UNDEFINED};
    #if MID_TO_END_DEPTH < USE_TT_DEPTH_THRESHOLD
        if (search->n_discs <= HW2 - USE_TT_DEPTH_THRESHOLD)
            transposition_table.get(search, hash_code, depth, &lower, &upper, moves);
    #else
        transposition_table.get(search, hash_code, depth, &lower, &upper, moves);
    #endif
    if (upper == lower)
        return upper;
    if (alpha < lower)
        return lower;
    if (upper <= alpha)
        return upper;
    #if USE_MID_MPC
        #if MID_FAST_NWS_DEPTH + 1 < USE_MPC_DEPTH
            if (depth >= USE_MPC_DEPTH){
                if (mpc(search, alpha, alpha + 1, depth, legal, is_end_search, &v, searching))
                    return v;
            }
        #else
            if (mpc(search, alpha, alpha + 1, depth, legal, is_end_search, &v, searching))
                return v;
        #endif
    #endif
    int best_move = TRANSPOSITION_TABLE_UNDEFINED;
    int g;
    const int canput = pop_count_ull(legal);
    std::vector<Flip_value> move_list(canput);
    int idx = 0;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&move_list[idx].flip, &search->board, cell);
        if (move_list[idx].flip.flip == search->board.opponent)
            return SCORE_MAX;
        ++idx;
    }
    int etc_done_idx = 0;
    #if USE_MID_ETC
        if (depth >= MID_ETC_DEPTH){
            if (etc_nws(search, move_list, depth, alpha, &v, &etc_done_idx))
                return v;
        }
    #endif
    #if USE_MID_MPC && false
        #if MID_FAST_NWS_DEPTH < USE_MPC_DEPTH
            if (depth >= USE_MPC_DEPTH + 1){
                if (enhanced_mpc(search, move_list, depth, alpha, alpha + 1, is_end_search, searching, &v))
                    return v;
            }
        #else
            if (enhanced_mpc(search, move_list, depth, alpha, alpha + 1, is_end_search, searching, &v))
                return v;
        #endif
    #endif
    move_list_evaluate_nws(search, move_list, moves, depth, alpha, searching);
    #if USE_ALL_NODE_PREDICTION
        const bool seems_to_be_all_node = predict_all_node(search, alpha, depth, LEGAL_UNDEFINED, is_end_search, searching);
    #else
        constexpr bool seems_to_be_all_node = false;
    #endif
    if (
        search->use_multi_thread && 
        #if MID_TO_END_DEPTH > YBWC_END_SPLIT_MIN_DEPTH
            ((depth - 1 >= YBWC_MID_SPLIT_MIN_DEPTH && !is_end_search) || (depth - 1 >= YBWC_END_SPLIT_MIN_DEPTH && is_end_search))
        #else
            depth - 1 >= YBWC_MID_SPLIT_MIN_DEPTH
        #endif
    ){
        int running_count = 0;
        std::vector<std::future<Parallel_task>> parallel_tasks;
        bool n_searching = true;
        for (int move_idx = 0; move_idx < canput - etc_done_idx && *searching && n_searching; ++move_idx){
            swap_next_best_move(move_list, move_idx, canput);
            #if USE_MID_ETC
                if (move_list[move_idx].flip.flip == 0ULL)
                    break;
            #endif
            eval_move(search, &move_list[move_idx].flip);
            search->move(&move_list[move_idx].flip);
                if (ybwc_split_nws(search, -alpha - 1, depth - 1, move_list[move_idx].n_legal, is_end_search, &n_searching, move_list[move_idx].flip.pos, move_idx, canput - etc_done_idx, running_count, seems_to_be_all_node, parallel_tasks)){
                    ++running_count;
                } else{
                    g = -nega_alpha_ordering_nws(search, -alpha - 1, depth - 1, false, move_list[move_idx].n_legal, is_end_search, searching);
                    if (v < g){
                        v = g;
                        best_move = move_list[move_idx].flip.pos;
                        if (alpha < v)
                            n_searching = false;
                    }
                    if (running_count){
                        ybwc_get_end_tasks(search, parallel_tasks, &v, &best_move, &running_count);
                        if (alpha < v)
                            n_searching = false;
                    }
                }
            search->undo(&move_list[move_idx].flip);
            eval_undo(search, &move_list[move_idx].flip);
        }
        if (running_count){
            if (!n_searching || !(*searching))
                ybwc_wait_all(search, parallel_tasks);
            else
                ybwc_wait_all_nws(search, parallel_tasks, &v, &best_move, &running_count, alpha, searching, &n_searching);
        }
    } else{
        for (int move_idx = 0; move_idx < canput - etc_done_idx && *searching; ++move_idx){
            swap_next_best_move(move_list, move_idx, canput);
            #if USE_MID_ETC
                if (move_list[move_idx].flip.flip == 0ULL)
                    break;
            #endif
            eval_move(search, &move_list[move_idx].flip);
            search->move(&move_list[move_idx].flip);
                g = -nega_alpha_ordering_nws(search, -alpha - 1, depth - 1, false, move_list[move_idx].n_legal, is_end_search, searching);
            search->undo(&move_list[move_idx].flip);
            eval_undo(search, &move_list[move_idx].flip);
            if (v < g){
                v = g;
                best_move = move_list[move_idx].flip.pos;
                if (alpha < v)
                    break;
            }
        }
    }
    if (*searching && global_searching)
        transposition_table.reg(search, hash_code, depth, alpha, alpha + 1, v, best_move);
    return v;
}

/*
int nega_alpha_ordering_nws_lazy_smp(Search *search, int alpha, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching){
    std::vector<std::future<int>> parallel_tasks;
    std::vector<Search> searches;
    int n_idle = thread_pool.get_n_idle();
    for (int i = 0; i < n_idle; ++i){
        Search n_search;
        n_search.init_board(&search->board);
        n_search.mpc_level = search->mpc_level;
        n_search.n_nodes = 0ULL;
        n_search.use_multi_thread = false;
        calc_features(&n_search);
        searches.emplace_back(n_search);
    }
    for (int i = 0; i < n_idle; ++i){
        bool pushed;
        parallel_tasks.emplace_back(thread_pool.push(&pushed, std::bind(&nega_alpha_ordering_nws, &searches[i], alpha, depth, skipped, legal, is_end_search, searching)));
        if (!pushed){
            parallel_tasks.pop_back();
            break;
        }
        if (n_idle <= thread_pool.get_n_idle())
            break;
    }
    std::cerr << parallel_tasks.size() + 1 << " parallel" << std::endl;
    int res = nega_alpha_ordering_nws(search, alpha, depth, skipped, legal, is_end_search, searching);
    uint64_t s = tim();
    for (std::future<int> &task: parallel_tasks)
        task.get();
    for (int i = 0; i < n_idle; ++i)
        search->n_nodes += searches[i].n_nodes;
    return res;
}
*/
