#pragma once
#include <string>
#include <format>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <ranges>
#include <algorithm>
#include <random>

using Position = std::pair<int, int>;
using Grid = std::vector<std::vector<std::string>>;

struct PositionHash {
    size_t operator()(const Position& p) const {
        return std::hash<int>()(p.first) ^ std::hash<int>()(p.second) << 1;
    }
};

class GridShuffler {
    Grid original_grid, shuffled_grid;
    std::vector<Position> non_empty_positions;
    std::unordered_map<std::string, std::unordered_set<std::string>> forbidden_neighbors;
    std::unordered_map<Position, std::vector<Position>, PositionHash> neighbors_map;
    std::unordered_map<Position, std::string, PositionHash> assignment;
    std::unordered_map<std::string, Position> original_positions;
    uint64_t rows, cols;

public:
    /**
     * @brief Construct a new Grid Shuffler object
     *
     * @param grid The input grid to be shuffled
     */
    explicit GridShuffler(const Grid& grid) :
        original_grid(grid),
        rows(grid.size()),
        cols(rows > 0? grid[0].size(): 0)
    {
        shuffled_grid = std::vector(rows, std::vector<std::string>(cols, ""));
        buildForbiddenNeighbors();
        buildNonEmptyPositions();
        buildNeighborsMap();
        buildOriginalPositions();
    }

private:
    /**
     * @brief Build the forbidden neighbors map based on the original grid
     *
     */
    void buildForbiddenNeighbors();

    /**
     * @brief Build the list of non-empty positions in the grid
     *
     */
    void buildNonEmptyPositions();

    /**
     * @brief Build the neighbors map for each non-empty position
     *
     */
    void buildNeighborsMap();

    /**
     * @brief Build the map of original positions for each digit
     *
     */
    void buildOriginalPositions();

    /**
     * @brief Check if assigning a digit to a position is valid
     *
     * @param pos The position to assign the digit to
     * @param digit The digit to assign
     * @param current_assignment The current assignment of digits to positions
     * @return true If the assignment is valid
     * @return false If the assignment is invalid
     */
    bool isValidAssignment(
        const Position& pos,
        const std::string& digit,
        const std::unordered_map<Position, std::string, PositionHash>& current_assignment);

    /**
     * @brief Backtracking algorithm to find a valid assignment of digits to positions
     *
     * @param current_assignment The current assignment of digits to positions
     * @param possible_digits The possible digits for each position
     * @return true If a valid assignment is found
     * @return false If no valid assignment is found
     */
    bool backtrack(std::unordered_map<Position, std::string, PositionHash>& current_assignment,
                   std::unordered_map<Position, std::vector<std::string>, PositionHash>& possible_digits
    ) {
        if (current_assignment.size() == non_empty_positions.size()) return true;

        const auto it = std::ranges::min_element(
            non_empty_positions,
            [&](const Position& a, const Position& b) {
                return !current_assignment.contains(a) &&
                       (current_assignment.contains(b) ||
                       possible_digits[a].size() < possible_digits[b].size());
            }
        );

        if (it == non_empty_positions.end() || current_assignment.contains(*it)) return false;
        const Position& pos = *it;

        for (const auto& digit : possible_digits[pos]) {
            if (isValidAssignment(pos, digit, current_assignment)) {
                current_assignment[pos] = digit;
                if (backtrack(current_assignment, possible_digits)) return true;
                current_assignment.erase(pos);
            }
        }

        return false; // 无有效分配，触发回溯
    }

public:
    /**
     * @brief Shuffle the grid according to the constraints
     *
     * @return true If the grid is successfully shuffled
     * @return false If the grid cannot be shuffled
     */
    bool shuffle(){
        std::unordered_map<Position, std::string, PositionHash> current_assignment;
        std::unordered_map<Position, std::vector<std::string>, PositionHash> possible_digits;

        static std::random_device rd;
        static std::mt19937 mt{rd()};

        const auto all_digits = forbidden_neighbors | std::views::keys | std::ranges::to<std::vector>();

        for (const auto& pos : non_empty_positions) {
            possible_digits[pos] = all_digits;
            std::ranges::shuffle(possible_digits[pos], mt);
        }

        if (backtrack(current_assignment, possible_digits)) {
            for (const auto& [pos, digit] : current_assignment) {
                const auto& [i, j] = pos;
                shuffled_grid[i][j] = digit;
            }
            return true;
        }
        return false;
    }

    /** Get the shuffled grid */
    [[nodiscard]]
    const Grid& getShuffledGrid() const
    { return shuffled_grid; }

    /**
     * @brief Validate the shuffled grid to ensure it meets all constraints
     *
     * @return true If the shuffled grid is valid
     * @return false If the shuffled grid is invalid
     */
    [[nodiscard]]
    bool validateResult() const {
        for (const auto& pos : non_empty_positions) {
            const std::string& digit = shuffled_grid[pos.first][pos.second];

            for (const auto& [fst, snd] : neighbors_map.at(pos)) {
                if (forbidden_neighbors.at(digit).contains(shuffled_grid[fst][snd])) {
                    return false;
                }
            }

            // 检查原始位置的分配
            if (auto it = original_positions.find(digit);
                it != original_positions.end() &&
                shuffled_grid[it->second.first][it->second.second] == digit
            ) {
                return false;
            }
        }

        return true;
    }
};

inline void GridShuffler::buildForbiddenNeighbors()  {
    std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            const std::string& tar = original_grid[i][j];
            if (tar.empty()) continue; // 跳过空位

            forbidden_neighbors[tar] = std::unordered_set<std::string>();

            for (auto& [fst, snd] : directions) {
                const int ni = i + fst;

                if (const int nj = j + snd;
                    ni >= 0 && ni < rows &&
                    nj >= 0 && nj < cols &&
                    !original_grid[ni][nj].empty()
                ) {
                    forbidden_neighbors[tar].insert(original_grid[ni][nj]);
                }
            }
        }
    }
}

inline void GridShuffler::buildNonEmptyPositions() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (!original_grid[i][j].empty()) {
                non_empty_positions.emplace_back(i, j);
            }
        }
    }
}

inline void GridShuffler::buildNeighborsMap() {
    std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    for (const auto& [i, j] : non_empty_positions) {
        Position pos = {i, j};
        neighbors_map[pos] = std::vector<Position>();

        for (auto& [fst, snd] : directions) {
            const int ni = i + fst;

            if (const int nj = j + snd;
                ni >= 0 && ni < rows &&
                nj >= 0 && nj < cols &&
                !original_grid[ni][nj].empty()
            ) {
                neighbors_map[pos].emplace_back(ni, nj);
            }
        }
    }
}

inline void GridShuffler::buildOriginalPositions() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            const std::string& tar = original_grid[i][j];
            if (tar.empty()) continue;

            original_positions[tar] = {i, j};
        }
    }
}

inline bool GridShuffler::isValidAssignment(
    const Position& pos,
    const std::string& digit,
    const std::unordered_map<Position, std::string, PositionHash>& current_assignment)
{
    // 检查相邻位置的分配
    for (const auto& neighbor : neighbors_map[pos]) {
        auto it = current_assignment.find(neighbor);
        if (it != current_assignment.end()) {
            const std::string& neighbor_digit = it->second;
            if (forbidden_neighbors[digit].contains(neighbor_digit)) {
                return false; // 相邻位置有冲突
            }
        }
    }

    // 检查原始位置的分配
    auto it = original_positions.find(digit);
    if (it != original_positions.end() && it->second == pos) {
        return false; // 原始位置有冲突
    }

    return true; // 分配有效
}

