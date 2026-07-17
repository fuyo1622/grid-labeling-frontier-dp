#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// Frontier state of a processed cell:
//   0: the cell contains 1
//   1: the cell contains 2 and is already adjacent to a 1
//   2: the cell contains 2 but still needs a 1 on its right or below

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    if (n <= 0) {
        cerr << "n must be positive\n";
        return 1;
    }

    vector<int> power3(n + 1, 1);
    for (int i = 1; i <= n; ++i) {
        if (power3[i - 1] > numeric_limits<int>::max() / 3) {
            cerr << "n is too large for this exact exponential algorithm\n";
            return 1;
        }
        power3[i] = power3[i - 1] * 3;
    }

    const int state_count = power3[n];
    const int INF = numeric_limits<int>::max() / 4;
    vector<int> dp(state_count, INF), next_dp(state_count, INF);

    // For every processed cell and resulting state, store
    //   (previous_state << 1) | (value - 1).
    // The low bit distinguishes value 1 from value 2.
    const uint32_t NO_PARENT = numeric_limits<uint32_t>::max();
    vector<vector<uint32_t>> parents;
    parents.reserve(n * n);

    // Before the first row there are no cells above it. State 1 is a neutral
    // virtual state: it neither requires the current cell to be 1 nor
    // falsely dominates it.
    int initial_state = 0;
    for (int c = 0; c < n; ++c) {
        initial_state += power3[c];
    }
    dp[initial_state] = 0;

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            fill(next_dp.begin(), next_dp.end(), INF);
            vector<uint32_t> step_parent(state_count, NO_PARENT);
            const int current_power = power3[c];
            const int left_power = (c == 0 ? 0 : power3[c - 1]);

            for (int state = 0; state < state_count; ++state) {
                if (dp[state] == INF) {
                    continue;
                }

                const int above = (state / current_power) % 3;
                const int left = (c == 0 ? 1 : (state / left_power) % 3);

                // Put 1 in the current cell. It dominates the cell above,
                // the cell to the left, and future cells right/below it.
                int next_state = state - above * current_power;
                if (c > 0 && left == 2) {
                    // The previously undominated left cell is now dominated.
                    next_state -= left_power;
                }
                if (dp[state] + 1 < next_dp[next_state]) {
                    next_dp[next_state] = dp[state] + 1;
                    step_parent[next_state] =
                        (static_cast<uint32_t>(state) << 1) | 0U;
                }

                // Put 2 in the current cell. If the cell above still needs
                // domination, this choice is impossible because none of its
                // future neighbors remain.
                if (above != 2) {
                    const bool dominated = (above == 0) || (c > 0 && left == 0);
                    const int new_digit = dominated ? 1 : 2;
                    next_state = state + (new_digit - above) * current_power;
                    if (dp[state] < next_dp[next_state]) {
                        next_dp[next_state] = dp[state];
                        step_parent[next_state] =
                            (static_cast<uint32_t>(state) << 1) | 1U;
                    }
                }
            }

            dp.swap(next_dp);
            parents.push_back(move(step_parent));
        }
    }

    // After the last row, no frontier cell may still be waiting for a 1
    // below it.
    int minimum_ones = INF;
    int best_state = -1;
    for (int state = 0; state < state_count; ++state) {
        if (dp[state] == INF) {
            continue;
        }

        int encoded = state;
        bool valid = true;
        for (int c = 0; c < n; ++c) {
            if (encoded % 3 == 2) {
                valid = false;
                break;
            }
            encoded /= 3;
        }

        if (valid && dp[state] < minimum_ones) {
            minimum_ones = dp[state];
            best_state = state;
        }
    }

    // Follow the stored transitions backward to reconstruct one optimum.
    vector<string> grid(n, string(n, '?'));
    int state = best_state;
    for (int step = n * n - 1; step >= 0; --step) {
        uint32_t code = parents[step][state];
        int value = static_cast<int>(code & 1U) + 1;
        grid[step / n][step % n] = static_cast<char>('0' + value);
        state = static_cast<int>(code >> 1);
    }

    const long long maximum_sum = 2LL * n * n - minimum_ones;
    cout << "maximum sum = " << maximum_sum << '\n';
    cout << "arrangement:\n";
    for (const string& row : grid) {
        cout << row << '\n';
    }
    return 0;
}
