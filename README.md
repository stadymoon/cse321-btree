# CSE321 Project #1 — B-Tree Index Structures

Implementation and experimental evaluation of B-Tree, B+-Tree, and B*-Tree.

## Environment

- Language: C++17
- Compiler: Apple Clang (macOS) / g++ (Linux)
- No external libraries required

## File Structure

```
.
├── btree.h / btree.cpp         # B-Tree
├── bplustree.h / bplustree.cpp # B+-Tree
├── bstartree.h / bstartree.cpp # B*-Tree
├── student.h                   # CSV loader + Student struct
├── main.cpp                    # Experiment runner
├── student.csv                 # 100,000 student records
└── results.csv                 # Experiment output
```

## Build

```bash
g++ -O2 -std=c++17 -o main main.cpp btree.cpp bplustree.cpp bstartree.cpp
```

## Run

```bash
./main student.csv
```

Results are printed to stdout and saved to `results.csv`.

## Experiments

| # | Description |
|---|---|
| 1 | Insert 100,000 records — time, splits, node utilization |
| 2 | Point search (10,000 random keys) — avg latency |
| 3 | Range query: avg GPA & height of male students, ID in [202000000, 202100000] |
| 4 | Delete 10% and 20% of records — time, post-deletion utilization |

All experiments run for d = 3, 5, 10.
