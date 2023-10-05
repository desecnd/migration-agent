#include <cstdint>
#include <cassert>
#include <bitset>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>
#include <algorithm>

#include "board.h"
#include "algorithms.h"

void test_minmax() {
    state s = generate_start_state(BOARD_SIZE);
    std::time_t start = std::time(nullptr);
    auto t_start = std::chrono::high_resolution_clock::now();
    while (1) {
        print_state(s);
        auto result = minmax(s);
        std::cout << result.score << " " << result.move << " " << result.nodes << std::endl;
        if (result.move == MOVE_INVALID) {
            break;
        }
        s = make_move(s, result.move);
    }
    auto t_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = t_end - t_start;
    auto dur_sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "Elapsed: " << dur_sec.count() << "s " << std::endl;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    std::cout << "Elapsed: " << dur_ms.count() << "ms " << std::endl;
}

void test_hashed_minmax() {
    state s = generate_start_state(BOARD_SIZE);
    std::time_t start = std::time(nullptr);
    auto t_start = std::chrono::high_resolution_clock::now();
    
    print_state(s);
    auto result = hashed_minmax(s, 0, 0);
    std::cout << result.score << " " << result.move << " " << result.nodes << std::endl;

    auto t_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = t_end - t_start;
    auto dur_sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "Elapsed: " << dur_sec.count() << "s " << std::endl;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    std::cout << "Elapsed: " << dur_ms.count() << "ms " << std::endl;
    
    std::cout << "Hashtable: (size=" << trans_table.size() << ")" << std::endl;
    
    /*
    for (auto p : trans_table) {
        if (p.second.collisions) {
            std::cout << "h: " << p.first << " => " << p.second.depth << " " << p.second.count << " " << p.second.nodes << " " << p.second.collisions << std::endl;
        }
    }
    */
}

void test_alfabeta() {
    state s = generate_start_state(BOARD_SIZE);
    std::time_t start = std::time(nullptr);
    auto t_start = std::chrono::high_resolution_clock::now();
    
    print_state(s);
    auto result = alfabeta(s, 0, 0, -100, 100);
    std::cout << result.score << " " << result.move << " " << result.nodes << std::endl;

    auto t_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = t_end - t_start;
    auto dur_sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "Elapsed: " << dur_sec.count() << "s " << std::endl;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    std::cout << "Elapsed: " << dur_ms.count() << "ms " << std::endl;
}

void test_hashed_alfabeta() {
    state s = generate_start_state(BOARD_SIZE);
    std::time_t start = std::time(nullptr);
    auto t_start = std::chrono::high_resolution_clock::now();
    
    print_state(s);
    std::cout << "Makng move: " << 24 << std::endl;
    s = make_move(s, 23);
    print_state(s);
    auto result = hashed_alfabeta(s, 0, 0, -100, 100);
    std::cout << result.score << " " << result.move << " " << result.nodes << std::endl;

    auto t_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = t_end - t_start;
    auto dur_sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "Elapsed: " << dur_sec.count() << "s " << std::endl;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    std::cout << "Elapsed: " << dur_ms.count() << "ms " << std::endl;
    std::cout << "Hashtable: (size=" << trans_tablev2.size() << ")" << std::endl;
}

void play_with_bot(int player = PLAYER_ORANGE) {
    assert(player == PLAYER_BLUE || player == PLAYER_ORANGE);
    state s = generate_start_state(BOARD_SIZE);
    
    std::cout << "You play as " << (player == PLAYER_ORANGE ? "ORANGE" : "BLUE") << std::endl;
    
    int turn = 0;
    while (1) {
        print_state(s);

        auto pos = get_moves(s);
        if (pos.empty()) {
            std::cout << "GAME OVER: " << (s.turn == PLAYER_ORANGE ? "BLUE" : "ORANGE") << " won" << std::endl;
            break;
        }
        
        if (player == s.turn) {
            std::cout << "Possible moves: [";
            for (int x : pos) {
                std::cout << x << ", ";
            }
            std::cout << "]\n";
            
            int move = -1;
            while (1) {
                std::cout << "Your move: ";
                std::cin >> move;
                if (std::find(pos.begin(), pos.end(), move) != pos.end()) {
                    break;
                }
                std::cout << "Invalid move. Try again\n";
            }
            s = make_move(s, move);
        }
        else {
            std::cout << "********************************************\n";
            std::cout << "AI is thinking..." << std::endl;
            std::time_t start = std::time(nullptr);
            auto t_start = std::chrono::high_resolution_clock::now();

            // auto result = hashed_alfabeta(s, turn, turn, -100, 100);
            auto result = hashed_minmax(s, turn, turn);
            std::cout << "AI moves: " << result.move << ", (score: " << result.score << ", nodes: " << result.nodes << ")" << std::endl;
            s = make_move(s, result.move);

            auto t_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> diff = t_end - t_start;
            auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
            std::cout << "Elapsed: " << dur_ms.count() << "ms " << std::endl;
            std::cout << "********************************************\n";
        }
        turn++;
    }

}


// creepy-bad in all the ways
void play_bot_with_bot(std::string p1, std::string p2) {

    state s = generate_start_state(BOARD_SIZE);
    std::cout << "Orange: " << p1 << std::endl;
    std::cout << "Blue: " << p2 << std::endl;
    
    int turn = 0;
    while (1) {
        print_state(s);

        auto pos = get_moves(s);
        if (pos.empty()) {
            std::cout << "GAME OVER: " << (s.turn == PLAYER_ORANGE ? p2 + " (BLUE)" : p1 + "(ORANGE)") << " won" << std::endl;
            break;
        }
        
        if (s.turn == PLAYER_ORANGE) {
            std::cout << "********************************************\n";
            std::cout << "AI is thinking..." << std::endl;
            std::time_t start = std::time(nullptr);
            auto t_start = std::chrono::high_resolution_clock::now();

            minmax_result result;
            std::string agent = p1;
            if (agent == "minmax") {
                result = minmax(s, turn);
            } else if (agent == "hashed_minmax"){
                result = hashed_minmax(s, turn, turn);
            } else if (agent == "hashed_ab") {
                result = hashed_alfabeta(s, turn, turn, -100, 100);
            }
            
            std::cout << "AI moves: " << result.move << ", (score: " << result.score << ", nodes: " << result.nodes << ")" << std::endl;
            s = make_move(s, result.move);

            auto t_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> diff = t_end - t_start;
            auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
            std::cout << "Elapsed: " << dur_ms.count() << "ms " << std::endl;
            std::cout << "********************************************\n";
        }
        else {
            std::cout << "********************************************\n";
            std::cout << "AI is thinking..." << std::endl;
            std::time_t start = std::time(nullptr);
            auto t_start = std::chrono::high_resolution_clock::now();

            minmax_result result;
            std::string agent = p2;
            if (agent == "minmax") {
                result = minmax(s, turn);
            } else if (agent == "hashed_minmax"){
                result = hashed_minmax(s, turn, turn);
            } else if (agent == "hashed_ab") {
                result = hashed_alfabeta(s, turn, turn, -100, 100);
            }
            std::cout << "AI moves: " << result.move << ", (score: " << result.score << ", nodes: " << result.nodes << ")" << std::endl;
            s = make_move(s, result.move);

            auto t_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> diff = t_end - t_start;
            auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
            std::cout << "Elapsed: " << dur_ms.count() << "ms " << std::endl;
            std::cout << "********************************************\n";
        }
        turn++;
    }
}


std::vector<int> generate_spiral(int n) {
    const int step_value[4] = {1, n, -1, -n};
    int step_size = n - 2;
    int stepping_dir = 0;
    int step_left = 3;
    
    std::vector<int> positions;
    positions.reserve(n * n - 2 * n + 1);
    int current_pos = n;

    positions.push_back(current_pos);
    while (step_size > 0) {
        for (int i = 0; i < step_size; i++) {
            current_pos += step_value[stepping_dir];
            positions.push_back(current_pos);
        }
        step_left--;
        stepping_dir++;
        if (stepping_dir >= 4) stepping_dir -= 4;

        if (step_left == 0) {
            step_left = 2;
            step_size--;
        }
    }
    return positions;
}

void test() {
    auto pos = generate_spiral(5);
    int map[5][5] = { 0 };
    
    for (int i = 0; i < pos.size(); i++) {
        int val = pos[i];
        std::cout << val << std::endl;
        
        map[val / 5][val % 5] = i;
    }
    
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
           std::cout << map[r][c] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    init();

    // test_minmax();
    // test_hashed_minmtax();
    // test_alfabeta();
    // test_hashed_alfabeta();
    // play_bot_with_bot("hashed_ab", "hashed_minmax");
    // test();
    // std::cout << "FINISH" << std::endl;
    play_with_bot(PLAYER_ORANGE);
    return 0;
}