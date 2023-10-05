#include <random>
#include <iostream>
#include <algorithm>
#include <cassert>

#include "board.h"
#include "algorithms.h"


std::mt19937_64 mt;

void generate_zobrist_keys(int seed) {
    mt.seed(seed);
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        zobrist_key[0][i] = mt();
        zobrist_key[1][i] = mt();
    }
}

void generate_moves_vector_spiral() {
    int n = BOARD_SIZE;
    const int step_value[4] = {1, n, -1, -n};
    int step_size = n - 2;
    int stepping_dir = 0;
    int step_left = 3;
    
    int current_pos = BOARD_SIZE;
    int next = 0;

    BOARD_MOVES[next++] = current_pos;
    while (step_size > 0) {
        for (int i = 0; i < step_size; i++) {
            current_pos += step_value[stepping_dir];
            BOARD_MOVES[next++] = current_pos;
        }
        step_left--;
        stepping_dir++;
        if (stepping_dir >= 4) stepping_dir -= 4;

        if (step_left == 0) {
            step_left = 2;
            step_size--;
        }
    }
    // std::cout << next << " " << BOARD_MAX_MOVES << std::endl;
    assert(next == BOARD_MAX_MOVES);
    std::cout << "Generated moves with spiral" << std::endl;
}

void generate_moves_vector_normal() {
    int next = 0;
    for (int r = 1; r < BOARD_SIZE; r++) 
    for (int c = 0; c < BOARD_SIZE - 1; c++) {
        int i = r * BOARD_SIZE + c;
        BOARD_MOVES[next++] = i;
    }
    assert(next == BOARD_MAX_MOVES);
    std::cout << "Generated moves with increasing sequence" << std::endl;
}
void generate_moves_vector_random() {
    generate_moves_vector_normal();
    std::shuffle(BOARD_MOVES, BOARD_MOVES + BOARD_MAX_MOVES, mt);
    std::cout << "Generated moves with random sequence" << std::endl;
}

void generate_moves_vector_spiral_reverse() {
    generate_moves_vector_spiral();
    std::reverse(BOARD_MOVES, BOARD_MOVES + BOARD_MAX_MOVES);
    std::cout << "Generated moves with reverse spiral" << std::endl;
}

void generate_moves_vector() {
    // generate_moves_vector_normal();
    // generate_moves_vector_random();
    // generate_moves_vector_spiral();
    generate_moves_vector_spiral_reverse();
}

void init(int seed) {
    generate_zobrist_keys(seed);
    generate_moves_vector();
}



minmax_result minmax(state s, int depth) {
    auto pos = get_moves(s);

    if (pos.empty()) {
        // std::cout << "Backtracking at depth: " << depth << std::endl; print_state(s);
        return { s.turn == PLAYER_ORANGE ? -1 : 1, MOVE_INVALID, 1ull };
    } 
    
    minmax_result result { .score = (s.turn == PLAYER_ORANGE ? -100 : 100), .move = MOVE_INVALID, .nodes = 1 };
    for (int move : pos) {
        state t = make_move(s, move);
        minmax_result child_result = minmax(t, depth + 1);
        
        result.nodes += child_result.nodes;
        if (s.turn == PLAYER_ORANGE) {
            if (child_result.score > result.score) {
                result.score = child_result.score;
                result.move = move;
            }
        }
        else {
            if (child_result.score < result.score) {
                result.score = child_result.score;
                result.move = move;
            }
        }

        if (depth == 0) {
            std::cout << "=======================\n";
            std::cout << "one-ahead for move: " << move << std::endl;
            print_state(t);
            std::cout << child_result.score << " " << child_result.move << " " << child_result.nodes << std::endl;
            std::cout << "=======================\n";
        }
    }
    return result;
}

minmax_result hashed_minmax(state s, int game_depth, int depth) {
    auto it = trans_table.find(s.z_hash);
    if (it != trans_table.end()) {
        ht_entry cache_result = it->second;
        
        if (cache_result.depth != (depth)) {
            it->second.collisions++;
            std::cout << "COLLISION!" << std::endl;
            std::cout << "stored: " << cache_result.depth << std::endl;
            std::cout << "i have: " << depth + game_depth << std::endl;
            print_state(s);
        }

        it->second.count++;
        return {
            cache_result.score, 
            cache_result.move,
            1,
        };
    }

    auto pos = get_moves(s);
    if (pos.empty()) {
        int score = s.turn == PLAYER_ORANGE ? -1 : 1;
        
        trans_table[s.z_hash] = {
            depth, score, MOVE_INVALID, 1, 1, 0
        };
        return { s.turn == PLAYER_ORANGE ? -1 : 1, MOVE_INVALID, 1 };
    } 
    
    minmax_result result { .score = (s.turn == PLAYER_ORANGE ? -100 : 100), .move = MOVE_INVALID, .nodes = 1 };
    for (int move : pos) {
        state t = make_move(s, move);
        minmax_result child_result = hashed_minmax(t, game_depth, depth + 1);
        
        
        result.nodes += child_result.nodes;
        if (s.turn == PLAYER_ORANGE) {
            if (child_result.score > result.score) {
                result.score = child_result.score;
                result.move = move;
            }
        }
        else {
            if (child_result.score < result.score) {
                result.score = child_result.score;
                result.move = move;
            }
        }

        if (depth == game_depth) {
            std::cout << "=======================\n";
            std::cout << "one-ahead for move: " << move << std::endl;
            print_state(t);
            std::cout << child_result.score << " " << child_result.move << " " << child_result.nodes << std::endl;
            std::cout << "=======================\n";
        }
    }
    
    if (trans_table.find(s.z_hash) != trans_table.end()) {
        std::cout << "INSERTING COLLISION!" << std::endl;
    }

    trans_table[s.z_hash] = {
        depth, result.score, result.move, 1, result.nodes, 0
    };
    
    /*
    std::cout << "ADDING HASHED STATE: "  << std::endl;
    print_state(s);
    std::cout << "d: " << depth << ", score: " << result.score << ", move: " << result.move << ", nodes below: " << result.nodes << std::endl;
    std::cout << "========"  << std::endl;
    */


    return result;
}

minmax_result alfabeta(state s, int game_depth, int depth, int alfa, int beta) {

    auto pos = get_moves(s);
    if (pos.empty()) {
        int score = s.turn == PLAYER_ORANGE ? -1 : 1;
        return { s.turn == PLAYER_ORANGE ? -1 : 1, MOVE_INVALID, 1 };
    } 
    
    minmax_result result { .score = (s.turn == PLAYER_ORANGE ? -1000 : 1000), .move = MOVE_INVALID, .nodes = 1 };
    for (int move : pos) {
        state t = make_move(s, move);
        minmax_result child_result = alfabeta(t, game_depth, depth + 1, alfa, beta);
        
        result.nodes += child_result.nodes;
        if (s.turn == PLAYER_ORANGE) {
            if (child_result.score > result.score) {
                result.score = child_result.score;
                result.move = move;
                alfa = std::max(alfa, result.score);
            }
        }
        else {
            if (child_result.score < result.score) {
                result.score = child_result.score;
                result.move = move;
                beta = std::min(beta, result.score);
            }
        }
        
        // no sense in doing one-step lookahead
        if (alfa >= beta) {
            break;
        }
    }
    return result;
}

minmax_result hashed_alfabeta(state s, int game_depth, int depth, int alfa, int beta) {
    auto it = trans_tablev2.find(s.z_hash);
    if (it != trans_tablev2.end()) {
        ht_entryv2 cache_result = it->second;
        
        if (cache_result.p1 != s.orange && cache_result.p2 != s.blue) {
            std::cout << "COLLISION!" << std::endl;
            print_state(s);
        }

        return {
            cache_result.score, 
            cache_result.move,
            1,
        };
    }

    auto pos = get_moves(s);
    if (pos.empty()) {
        int score = s.turn == PLAYER_ORANGE ? -1 : 1;
        
        trans_tablev2[s.z_hash] = {
            score, MOVE_INVALID, s.orange, s.blue
        };
        return { s.turn == PLAYER_ORANGE ? -1 : 1, MOVE_INVALID, 1 };
    } 
    
    minmax_result result { .score = (s.turn == PLAYER_ORANGE ? -1000 : 1000), .move = MOVE_INVALID, .nodes = 1 };
    for (int move : pos) {
        state t = make_move(s, move);
        minmax_result child_result = hashed_alfabeta(t, game_depth, depth + 1, alfa, beta);
        
        
        result.nodes += child_result.nodes;
        if (s.turn == PLAYER_ORANGE) {
            if (child_result.score > result.score) {
                result.score = child_result.score;
                result.move = move;
                alfa = std::max(alfa, result.score);
            }
        }
        else {
            if (child_result.score < result.score) {
                result.score = child_result.score;
                result.move = move;
                beta = std::min(beta, result.score);
            }
        }

        if (alfa >= beta) {
            break;
        }
    }

    if (trans_tablev2.find(s.z_hash) != trans_tablev2.end()) {
        std::cout << "INSERTING COLLISION!" << std::endl;
    }

    trans_tablev2[s.z_hash] = {
        result.score, result.move, s.orange, s.blue
    };
    
    return result;
}