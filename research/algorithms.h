#pragma once

#include <unordered_map>
#include "board.h"

struct minmax_result {
    int score = 0;
    int move = 0;
    int nodes = 0;
};

struct ht_entry {
    int depth;
    int score;
    int move;
    int count;
    int nodes;
    int collisions;
};

struct ht_entryv2 {
    int score;
    int move;
    uint64_t p1;
    uint64_t p2;
};

inline std::unordered_map<hash_t, ht_entry> trans_table;
inline std::unordered_map<hash_t, ht_entryv2> trans_tablev2;

void init(int seed = 0xdeadbeef);
minmax_result minmax(state s, int depth = 0);
minmax_result hashed_minmax(state s, int game_depth, int depth = 0);
minmax_result alfabeta(state s, int game_depth, int depth, int alfa, int beta);
minmax_result hashed_alfabeta(state s, int game_depth, int depth, int alfa, int beta);