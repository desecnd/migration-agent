#pragma once

#include <unordered_map>
#include <vector>
    
#define PLAYER_ORANGE 0
#define PLAYER_BLUE 1
#define MOVE_INVALID -1

const int BOARD_SIZE = 7;
const int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;
const int BOARD_MAX_MOVES = (BOARD_SIZE - 1) * (BOARD_SIZE - 1);
using hash_t = uint64_t;

inline hash_t zobrist_key[2][BOARD_CELLS];
inline int BOARD_MOVES[BOARD_MAX_MOVES];

struct state {
    uint64_t orange = 0ull, blue = 0ull;
    int turn = PLAYER_ORANGE;
    hash_t z_hash = 0ull;
};

void print_state(state s);
state generate_start_state(int n);

std::vector<int> get_moves(state s);
state make_move(state s, int move);