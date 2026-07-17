# Grid Labeling Frontier DP

Exact C++17 and Python solvers for two neighbor-constrained labeling problems
on an `n x n` square grid. The implementations use frontier dynamic
programming and can reconstruct optimal arrangements.

Adjacency always means sharing an edge: up, down, left, or right. Diagonal
cells are not adjacent. These are exact exponential-width algorithms aimed
at small grids and algorithmic experimentation.

## Repository contents

| File | Variant | Output |
|:--|:--|:--|
| `three_values_solution.cpp` | Values 1, 2, 3 | Maximum sum and an optimal arrangement |
| `three_values_solution.py` | Values 1, 2, 3 | Maximum sum |
| `two_values_solution.cpp` | Values 1, 2 | Maximum sum and an optimal arrangement |
| `two_values_solution.py` | Values 1, 2 | Maximum sum and an optimal arrangement |

## Problem 1: values 1, 2, and 3

Put one value in every grid cell, subject to these rules:

- A cell containing `1` has no restriction.
- A cell containing `2` must have at least one adjacent `1`.
- A cell containing `3` must have at least one adjacent `1` and at least one
  adjacent `2`. The two supporting neighbors must therefore be different
  cells.

The objective is to maximize the sum of all cell values.

If `A` is the set of 1-cells and `B` is the set of 2-cells, the problem also
has the following graph interpretation:

- `A` dominates the entire grid graph.
- `B` dominates the graph remaining after `A` is removed.
- Relative to an all-3 grid, the lost value (the penalty) is
  `2|A| + |B|`.

The repository provides two exact implementations:

- `three_values_solution.cpp` prints the maximum sum and one optimal
  arrangement.
- `three_values_solution.py` prints the maximum sum.

## Problem 2: values 1 and 2

This variant removes value `3`:

- A cell containing `1` has no restriction.
- A cell containing `2` must have at least one adjacent `1`.

Again, the objective is to maximize the grid sum.

The 1-cells now form an ordinary dominating set of the grid graph. If the
smallest such set contains `gamma(n)` cells, then

```text
maximum sum = 2n^2 - gamma(n).
```

Both implementations compute the answer by exact frontier dynamic
programming and reconstruct one optimal arrangement:

- `two_values_solution.cpp`
- `two_values_solution.py`

## Frontier dynamic programming

All four solvers process cells from left to right and top to bottom. At any
time, the frontier contains the most recently processed cell in each column.
A state records only information that can affect unprocessed cells.

### Three-value frontier states

Each frontier cell has one of seven states:

```text
value 1: no condition
value 2: missing 1 / already has 1
value 3: has neither / has only 1 / has only 2 / has both
```

When a new cell is processed:

1. Its value is tried as `1`, `2`, and `3`.
2. It becomes the last possible unprocessed neighbor of the cell above, so
   that upper cell must now satisfy its rule.
3. The state of the cell to the left is updated with the new neighbor.
4. A new frontier state is created for the current cell using its upper and
   left neighbors.
5. Among transitions producing the same frontier, only the best sum is kept.

After the final row, every remaining frontier cell is checked because no
lower neighbor exists.

### Two-value frontier states

Only three states per frontier cell are needed:

```text
0: the cell is 1
1: the cell is 2 and is already adjacent to a 1
2: the cell is 2 and still needs a 1 on its right or below
```

The DP minimizes the number of 1-cells. Putting a 1 updates pending cells to
its left or above; putting a 2 is rejected if the cell above is still
pending. Parent transitions are retained so that one optimal grid can be
reconstructed.

## Correctness outline

The frontier state is a complete description of how the processed part can
interact with the unprocessed part.

- Every possible value of the next cell is considered.
- A processed cell is discarded exactly when its final possible neighbor
  cannot satisfy its requirement.
- States with the same frontier have identical possible continuations, so
  retaining only the best partial objective cannot remove an optimum.
- The final frontier check enforces the conditions on the bottom row.

By induction over the row-major processing order, every valid grid is
represented by one DP path and every surviving complete path is a valid
grid. Therefore the best surviving value is the global optimum. Following
the stored parent transitions recovers a grid attaining it.

## Complexity

Let the width be `n`.

| Variant | Time | Score-layer memory |
|:--|:--|:--|
| Values 1, 2, 3 | `O(n^2 * 7^n)` | `O(7^n)` |
| Values 1, 2 | `O(n^2 * 3^n)` | `O(3^n)` |

Programs that output an arrangement also retain parent information. In the
worst case this adds `O(n^2 * 7^n)` memory for the three-value solver or
`O(n^2 * 3^n)` for the two-value solver.

These are exact exponential-width algorithms. The C++ versions are the
better choice for larger inputs; the Python versions are intended mainly for
small grids and experimentation.

## Building and running

All programs read one positive integer `n` from standard input.

### C++

```bash
g++ -std=c++17 -O2 three_values_solution.cpp -o three_values_solution
echo 5 | ./three_values_solution

g++ -std=c++17 -O2 two_values_solution.cpp -o two_values_solution
echo 5 | ./two_values_solution
```

The C++ programs print:

```text
maximum sum = ...
arrangement:
...
```

### Python

```bash
echo 5 | python three_values_solution.py
echo 5 | python two_values_solution.py
```

`three_values_solution.py` prints only the maximum sum. The two-value Python
program prints both the maximum sum and one optimal arrangement.

## Verified small results

```text
n                    1   2   3   4   5   6    7
values 1, 2, 3       1   7  19  36  55  80  111
values 1, 2          1   6  15  28  43  62   86
```
