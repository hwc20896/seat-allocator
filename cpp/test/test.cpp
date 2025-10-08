#include <gtest/gtest.h>
#include "../grid_shuffler_alg.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <print>
using namespace std;

const Grid grid = {{"1","2","3"},{"4","5","6"},{"7","8","9"}};

TEST(GridAlgorithmTest, BasicTest) {
    GridShuffler shuffler(grid);
    ASSERT_TRUE(shuffler.shuffle());
}

TEST(GridAlgorithmTest, WithEmptyCells) {
    const Grid grid = {{"1","2","3"},{"4","","6"},{"7","",""}};
    GridShuffler shuffler(grid);
    ASSERT_TRUE(shuffler.shuffle());
}

TEST(GridAlgorithmTest, AssertNoDuplicates) {
    std::println("{}", grid);
    GridShuffler shuffler(grid);
    shuffler.shuffle();
    auto shuffled = shuffler.getShuffledGrid();
    std::println("{}", shuffled);

    const auto before = grid | std::views::join | std::ranges::to<std::vector>();
    auto after = shuffled | std::views::join | std::ranges::to<std::vector>();
    std::ranges::sort(after);
    ASSERT_EQ(before, after);
}

TEST(GridAlgorithmTest, ValidateResult) {
    GridShuffler shuffler(grid);
    shuffler.shuffle();
    ASSERT_TRUE(shuffler.validateResult());
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}