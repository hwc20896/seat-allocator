#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <iostream>
#include <string>
#include <format>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include <random>
#include <limits>
#include <ranges>

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
    explicit GridShuffler(const Grid& grid);

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
                   std::unordered_map<Position, std::vector<std::string>, PositionHash>& possible_digits);

public:
    /**
     * @brief Shuffle the grid according to the constraints
     *
     * @return true If the grid is successfully shuffled
     * @return false If the grid cannot be shuffled
     */
    bool shuffle();

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
    bool validateResult() const;
};

namespace py = pybind11;

PYBIND11_MODULE(grid_shuffler, m) {
    m.doc() = "Grid shuffler algorithm for Python";

    py::class_<GridShuffler>(m, "GridShuffler")
        .def(py::init<const Grid&>())
        .def("shuffle", &GridShuffler::shuffle)
        .def("get_shuffled_grid", &GridShuffler::getShuffledGrid)
        .def("validate_result", &GridShuffler::validateResult);
}

//   Content
GridShuffler::GridShuffler(const Grid& grid) :
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

void GridShuffler::buildForbiddenNeighbors()  {
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

void GridShuffler::buildNonEmptyPositions() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (!original_grid[i][j].empty()) {
                non_empty_positions.emplace_back(i, j);
            }
        }
    }
}

void GridShuffler::buildNeighborsMap() {
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

void GridShuffler::buildOriginalPositions() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            const std::string& tar = original_grid[i][j];
            if (tar.empty()) continue;

            original_positions[tar] = {i, j};
        }
    }
}

bool GridShuffler::isValidAssignment(
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
    if (it != original_positions.end()) {
        const Position& original_pos = it->second;
        if (auto assigned_it = current_assignment.find(original_pos); assigned_it != current_assignment.end() && assigned_it->second == digit) {
            return false; // 原始位置有冲突
        }
    }

    return true; // 分配有效
}

bool GridShuffler::backtrack(
    std::unordered_map<Position, std::string, PositionHash>& current_assignment,
    std::unordered_map<Position, std::vector<std::string>, PositionHash>& possible_digits)
{
    if (current_assignment.size() == non_empty_positions.size()) {
        return true; // 所有位置都已分配
    }

    // 选择下一个位置（使用最少剩余值启发式）
    Position next_pos;
    size_t min_options = std::numeric_limits<size_t>::max();

    for (const auto& pos : non_empty_positions) {
        if (!current_assignment.contains(pos)) {
            if (size_t options_count = possible_digits[pos].size();
                options_count < min_options
            ) {
                min_options = options_count;
                next_pos = pos;
            }
        }
    }

    // 尝试为选定位置分配每个可能的数字
    for (const auto& digit : possible_digits[next_pos]) {
        if (isValidAssignment(next_pos, digit, current_assignment)) {
            current_assignment[next_pos] = digit;

            // 递归调用
            if (backtrack(current_assignment, possible_digits)) {
                return true;
            }

            // 回溯
            current_assignment.erase(next_pos);
        }
    }

    return false; // 无有效分配，触发回溯
}

bool GridShuffler::shuffle() {
    std::unordered_map<Position, std::string, PositionHash> current_assignment;
    std::unordered_map<Position, std::vector<std::string>, PositionHash> possible_digits;

    static std::random_device rd;
    static std::mt19937 mt{rd()};

    // 初始化每个位置的可能数字列表
    for (const auto& pos : non_empty_positions) {
        possible_digits[pos] = std::vector<std::string>();
        for (const auto& digit: forbidden_neighbors | std::views::keys) {
            possible_digits[pos].push_back(digit);
        }
        // 打乱可能数字的顺序以增加随机性
        std::ranges::shuffle(possible_digits[pos], mt);
    }

    if (backtrack(current_assignment, possible_digits)) {
        // 将结果填充到 shuffled_grid 中
        for (const auto& [pos, digit] : current_assignment) {
            const auto& [i, j] = pos;
            shuffled_grid[i][j] = digit;
        }
        return true; // 成功找到一个有效的打乱方案
    }
    return false; // 未找到有效的打乱方案
}

bool GridShuffler::validateResult() const {
    // 检查每个非空位置的邻居关系
    for (const auto& pos : non_empty_positions) {
        const auto& [i, j] = pos;
        const std::string& digit = shuffled_grid[i][j];

        for (const auto& neighbor : neighbors_map.at(pos)) {
            if (const auto& [ni, nj] = neighbor;
                forbidden_neighbors.at(digit).contains(shuffled_grid[ni][nj])
            ) {
                return false; // 相邻位置有冲突
            }
        }

        // 检查原始位置的分配
        if (auto it = original_positions.find(digit);
            it != original_positions.end()
        ) {
            const auto&[fst, snd] = it->second;
            if (shuffled_grid[fst][snd] == digit) {
                return false; // 原始位置有冲突
            }
        }
    }

    return true; // 所有检查通过，结果有效
}

