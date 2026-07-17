"""Exact solver for the variant with only values 1 and 2.

Input:  n
Output: maximum possible sum and one optimal n by n arrangement

A 2-cell must have a four-directionally adjacent 1-cell.  The frontier DP
minimizes the number of 1-cells; the maximum sum is then 2*n*n minus that
minimum. Parent transitions are retained to reconstruct one optimum.
"""

from __future__ import annotations

import sys


# Frontier state for one processed cell:
#   0: the cell is 1
#   1: the cell is 2 and is already adjacent to a 1
#   2: the cell is 2 and still needs a 1 on its right or below


def solve(n: int) -> tuple[int, list[str]]:
    if n <= 0:
        raise ValueError("n must be positive")

    power3 = [1] * (n + 1)
    for i in range(1, n + 1):
        power3[i] = 3 * power3[i - 1]

    # State 1 is neutral for the virtual cells above the first row: it does
    # not require domination and does not falsely dominate the first row.
    initial_state = sum(power3[column] for column in range(n))
    dp: dict[int, int] = {initial_state: 0}
    parents: list[dict[int, tuple[int, int]]] = []

    for _row in range(n):
        for column in range(n):
            next_dp: dict[int, int] = {}
            step_parents: dict[int, tuple[int, int]] = {}
            current_power = power3[column]
            left_power = 0 if column == 0 else power3[column - 1]

            for state, old_ones in dp.items():
                above = (state // current_power) % 3
                left = 1 if column == 0 else (state // left_power) % 3

                # Put 1 in the current cell.  It satisfies an undominated
                # cell above and changes an undominated left cell to state 1.
                new_state = state - above * current_power
                if column > 0 and left == 2:
                    new_state -= left_power

                new_cost = old_ones + 1
                previous = next_dp.get(new_state)
                if previous is None or new_cost < previous:
                    next_dp[new_state] = new_cost
                    step_parents[new_state] = (state, 1)

                # Put 2 in the current cell.  This is impossible if the cell
                # above still needs a 1, since all its neighbors are decided.
                if above != 2:
                    dominated = above == 0 or (column > 0 and left == 0)
                    new_digit = 1 if dominated else 2
                    new_state = state + (new_digit - above) * current_power

                    previous = next_dp.get(new_state)
                    if previous is None or old_ones < previous:
                        next_dp[new_state] = old_ones
                        step_parents[new_state] = (state, 2)

            dp = next_dp
            parents.append(step_parents)

    best = n * n + 1
    best_state = -1
    for state, number_of_ones in dp.items():
        encoded = state
        valid = True
        for _column in range(n):
            if encoded % 3 == 2:
                valid = False
                break
            encoded //= 3
        if valid:
            if number_of_ones < best:
                best = number_of_ones
                best_state = state

    # Follow the chosen transitions backward to recover one optimal grid.
    grid = [["?"] * n for _ in range(n)]
    state = best_state
    for step in range(n * n - 1, -1, -1):
        previous_state, value = parents[step][state]
        row, column = divmod(step, n)
        grid[row][column] = str(value)
        state = previous_state

    maximum = 2 * n * n - best
    return maximum, ["".join(row) for row in grid]


def minimum_number_of_ones(n: int) -> int:
    maximum, _grid = solve(n)
    return 2 * n * n - maximum



def maximum_sum(n: int) -> int:
    maximum, _grid = solve(n)
    return maximum


def main() -> None:
    data = sys.stdin.buffer.read().split()
    if len(data) != 1:
        raise SystemExit("expected one positive integer n")

    n = int(data[0])
    maximum, grid = solve(n)
    print(f"maximum sum = {maximum}")
    print("arrangement:")
    print(*grid, sep="\n")


if __name__ == "__main__":
    main()
