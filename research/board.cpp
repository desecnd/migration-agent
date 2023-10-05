#include <iostream>
#include <cassert>

#include "board.h"


void print_state(state s) {
    // std::cout << "Orange: " << std::bitset<64>(s.orange) << std::endl;
    // std::cout << "Blue:   " << std::bitset<64>(s.blue) << std::endl;
    std::cout << "Turn: " << (s.turn == PLAYER_ORANGE ? "ORANGE" : "BLUE") << std::endl;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (s.orange & (1ull << (r * BOARD_SIZE + c)))  {
                std::cout << "O";
            }
            else if (s.blue & (1ull << (r * BOARD_SIZE + c))) {
                std::cout << "B";
            }
            else {
                std::cout << ".";
            }
        }
        std::cout << '\n';
    }
    std::cout.flush();
}

state generate_start_state(int n) {
    assert(n >= 4 && n <= 8);
    state start;
    start.turn = PLAYER_ORANGE;
    int k =  (n + 1) / 2 - 1;
    for (int c = 0; c < k; c++) {
        for (int r = c + 1; r < n - c - 1; r++) {
            start.orange |= 1ull << (r * n + c);
            start.z_hash ^= zobrist_key[PLAYER_ORANGE][r * n + c];
            start.blue |= 1ull << ((n - c - 1) * n + r);
            start.z_hash ^= zobrist_key[PLAYER_BLUE][(n - c - 1) * n + r];
        }
    }
    return start;
}



std::vector<int> get_moves(state s) {
    uint64_t s_or = s.orange | s.blue;
    // std::cout << "S_or:   " << std::bitset<64>(s_or) << std::endl;
    std::vector<int> positions;
    
    // assert BOARD MOVES does not contain 0 row and n - 1 column
    for (int index = 0; index < BOARD_MAX_MOVES; index++) {
        int i = BOARD_MOVES[index];

        if (s.turn == PLAYER_ORANGE && (s.orange & (1ull << i)) && !(s_or & (1ull << (i + 1)))) {
            positions.push_back(i);
        }
        else if (s.turn == PLAYER_BLUE && (s.blue & (1ull << i)) && !(s_or & (1ull << (i - BOARD_SIZE)))) {
            positions.push_back(i);
        }
    }
    return positions;
}


state make_move(state s, int move) {
    uint64_t s_or = s.orange | s.blue;
    assert(move >= 0 && move <= BOARD_CELLS);
    if (s.turn == PLAYER_ORANGE) {
        assert(s.orange & (1ull << move));
        assert(!(s_or & (1ull << (move + 1))));
        
        s.orange &= ~(1ull << move);
        s.orange |= 1ull << (move + 1);
        
        s.z_hash ^= zobrist_key[PLAYER_ORANGE][move];
        s.z_hash ^= zobrist_key[PLAYER_ORANGE][move + 1];
    } else {
        assert(s.blue & (1ull << move));
        assert(!(s_or & (1ull << (move - BOARD_SIZE))));

        s.blue &= ~(1ull << move);
        s.blue |= 1ull << (move - BOARD_SIZE);

        s.z_hash ^= zobrist_key[PLAYER_BLUE][move];
        s.z_hash ^= zobrist_key[PLAYER_BLUE][move - BOARD_SIZE];
    }
    
    s.turn ^= 1;
    return s;
}
