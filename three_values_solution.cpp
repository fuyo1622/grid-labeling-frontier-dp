#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Frontier state for one cell:
//   0       : value 1
//   1, 2    : value 2, without/with an adjacent 1
//   3..6    : value 3, plus two bits saying whether adjacent 1 and 2 exist
//   7       : empty (only used before the first row is filled)

int cellValue(int state) {
    if (state == 0) return 1;
    if (state <= 2) return 2;
    return 3;
}

int addNeighbor(int state, int neighborValue) {
    if (state == 0) return state;  // A 1-cell has no condition.

    if (state <= 2) {
        return neighborValue == 1 ? 2 : state;
    }

    int flags = state - 3;
    if (neighborValue == 1) flags |= 1;
    if (neighborValue == 2) flags |= 2;
    return 3 + flags;
}

bool isValid(int state) {
    return state == 0                 // value 1
        || state == 2                 // value 2 with an adjacent 1
        || state == 6;                // value 3 with adjacent 1 and 2
}

int makeState(int value, int upperValue, int leftValue) {
    int state = (value == 1 ? 0 : value == 2 ? 1 : 3);
    if (upperValue != 0) state = addNeighbor(state, upperValue);
    if (leftValue != 0) state = addNeighbor(state, leftValue);
    return state;
}

struct Parent {
    uint64_t previousFrontier;
    int value;
};

struct Result {
    int maximumSum;
    vector<string> grid;
};

Result solve(int n) {
    // Each frontier cell occupies three bits because it has eight states.
    uint64_t emptyFrontier = 0;
    for (int col = 0; col < n; ++col) {
        emptyFrontier |= 7ULL << (3 * col);
    }

    // frontier encoding -> maximum sum obtained so far
    unordered_map<uint64_t, int> dp, nextDp;
    dp[emptyFrontier] = 0;

    // parents[step][frontier] tells us which state and value produced this
    // optimal frontier at that step.
    vector<unordered_map<uint64_t, Parent>> parents;
    parents.reserve(n * n);

    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < n; ++col) {
            nextDp.clear();
            nextDp.reserve(dp.size() * 2);
            unordered_map<uint64_t, Parent> stepParents;
            stepParents.reserve(dp.size() * 2);

            const int shift = 3 * col;
            const uint64_t currentMask = 7ULL << shift;
            const uint64_t leftMask =
                (col == 0 ? 0 : 7ULL << (shift - 3));

            for (const auto& [frontier, sum] : dp) {
                int upperState = static_cast<int>((frontier >> shift) & 7);
                int leftState = (col == 0)
                    ? 7
                    : static_cast<int>((frontier >> (shift - 3)) & 7);

                int upperValue =
                    (upperState == 7 ? 0 : cellValue(upperState));
                int leftValue =
                    (leftState == 7 ? 0 : cellValue(leftState));

                for (int value = 1; value <= 3; ++value) {
                    // The new cell is the last unknown neighbor of the upper
                    // cell, so the upper cell must now satisfy its condition.
                    if (upperState != 7
                        && !isValid(addNeighbor(upperState, value))) {
                        continue;
                    }

                    int newLeftState = (leftState == 7)
                        ? 7
                        : addNeighbor(leftState, value);
                    int newCurrentState =
                        makeState(value, upperValue, leftValue);

                    uint64_t newFrontier =
                        (frontier & ~currentMask)
                        | (static_cast<uint64_t>(newCurrentState) << shift);

                    if (col != 0) {
                        newFrontier =
                            (newFrontier & ~leftMask)
                            | (static_cast<uint64_t>(newLeftState)
                               << (shift - 3));
                    }

                    int newSum = sum + value;
                    auto it = nextDp.find(newFrontier);
                    if (it == nextDp.end() || newSum > it->second) {
                        nextDp[newFrontier] = newSum;
                        stepParents[newFrontier] = {frontier, value};
                    }
                }
            }

            dp.swap(nextDp);
            parents.push_back(move(stepParents));
        }
    }

    // Bottom-row cells have no lower neighbor, so validate them here.
    int answer = 0;
    uint64_t bestFrontier = 0;
    for (const auto& [frontier, sum] : dp) {
        bool validGrid = true;
        for (int col = 0; col < n; ++col) {
            int state = static_cast<int>((frontier >> (3 * col)) & 7);
            if (!isValid(state)) {
                validGrid = false;
                break;
            }
        }
        if (validGrid && sum > answer) {
            answer = sum;
            bestFrontier = frontier;
        }
    }

    // Follow the stored parents backwards to recover one optimal grid.
    vector<string> grid(n, string(n, '0'));
    uint64_t frontier = bestFrontier;
    for (int step = n * n - 1; step >= 0; --step) {
        const Parent& parent = parents[step].at(frontier);
        grid[step / n][step % n] = static_cast<char>('0' + parent.value);
        frontier = parent.previousFrontier;
    }

    return {answer, grid};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1 || n > 20) {
        cerr << "Expected 1 <= n <= 20.\n";
        return 1;
    }

    Result result = solve(n);
    cout << "maximum sum = " << result.maximumSum << '\n';
    cout << "arrangement:\n";
    for (const string& row : result.grid) {
        cout << row << '\n';
    }
    return 0;
}
