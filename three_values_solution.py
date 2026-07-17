"""Exact solver for the original problem with values 1, 2, and 3.

Input:  n
Output: maximum possible sum of an n by n grid

The algorithm is a frontier dynamic program.  Each frontier cell stores its
value and which required neighbor values it has already seen.
"""

from __future__ import annotations

import sys


# Frontier state for one cell:
#   0: value 1
#   1: value 2, has not seen a neighboring 1
#   2: value 2, has seen a neighboring 1
#   3: value 3, has seen neither 1 nor 2
#   4: value 3, has seen 1 only
#   5: value 3, has seen 2 only
#   6: value 3, has seen both 1 and 2
#   7: no cell (used only before the first row)

CELL_VALUE = (1, 2, 2, 3, 3, 3, 3, 0)
IS_VALID = (True, False, True, False, False, False, True, False)


def add_neighbor(state: int, neighbor_value: int) -> int:
    """Update a frontier state after exposing one adjacent cell."""
    if state == 0 or state == 7:
        return state

    if state <= 2:
        return 2 if neighbor_value == 1 else state

    flags = state - 3
    if neighbor_value == 1:
        flags |= 1
    elif neighbor_value == 2:
        flags |= 2
    return 3 + flags


# Small transition tables save considerable work in the inner DP loop.
ADD_NEIGHBOR = tuple(
    tuple(add_neighbor(state, value) for value in range(4))
    for state in range(8)
)


def make_state(value: int, upper_value: int, left_value: int) -> int:
    state = 0 if value == 1 else 1 if value == 2 else 3
    if upper_value:
        state = ADD_NEIGHBOR[state][upper_value]
    if left_value:
        state = ADD_NEIGHBOR[state][left_value]
    return state


MAKE_STATE = tuple(
    tuple(
        tuple(make_state(value, upper, left) for left in range(4))
        for upper in range(4)
    )
    for value in range(4)
)


def reverse_frontier(frontier: int, width: int) -> int:
    """Reflect a complete row to merge horizontally symmetric states."""
    result = 0
    for column in range(width):
        digit = (frontier >> (3 * column)) & 7
        result |= digit << (3 * (width - 1 - column))
    return result


def maximum_sum(n: int) -> int:
    if n <= 0:
        raise ValueError("n must be positive")

    empty_frontier = sum(7 << (3 * column) for column in range(n))
    dp: dict[int, int] = {empty_frontier: 0}

    for _row in range(n):
        for column in range(n):
            next_dp: dict[int, int] = {}
            shift = 3 * column
            current_mask = 7 << shift
            left_shift = shift - 3
            left_mask = 0 if column == 0 else 7 << left_shift

            for frontier, old_sum in dp.items():
                upper_state = (frontier >> shift) & 7
                left_state = 7 if column == 0 else (frontier >> left_shift) & 7
                upper_value = CELL_VALUE[upper_state]
                left_value = CELL_VALUE[left_state]

                for value in (1, 2, 3):
                    # The new cell is the final possible neighbor of the cell
                    # above.  That upper cell must now satisfy its rule.
                    if upper_state != 7:
                        finished_upper = ADD_NEIGHBOR[upper_state][value]
                        if not IS_VALID[finished_upper]:
                            continue

                    new_left = ADD_NEIGHBOR[left_state][value]
                    new_current = MAKE_STATE[value][upper_value][left_value]

                    new_frontier = (
                        (frontier & ~current_mask) | (new_current << shift)
                    )
                    if column != 0:
                        new_frontier = (
                            (new_frontier & ~left_mask)
                            | (new_left << left_shift)
                        )

                    # Once a row is complete, its reflection has an identical
                    # remaining subproblem, so retain one orientation only.
                    if column == n - 1:
                        new_frontier = min(
                            new_frontier,
                            reverse_frontier(new_frontier, n),
                        )

                    new_sum = old_sum + value
                    previous = next_dp.get(new_frontier)
                    if previous is None or new_sum > previous:
                        next_dp[new_frontier] = new_sum

            dp = next_dp

    answer = 0
    for frontier, grid_sum in dp.items():
        if all(
            IS_VALID[(frontier >> (3 * column)) & 7]
            for column in range(n)
        ):
            answer = max(answer, grid_sum)
    return answer


def main() -> None:
    data = sys.stdin.buffer.read().split()
    if len(data) != 1:
        raise SystemExit("expected one positive integer n")

    n = int(data[0])
    print(maximum_sum(n))


if __name__ == "__main__":
    main()
