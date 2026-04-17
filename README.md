# CSC209 Project — Distributed Text Analyser

**Group members:** Jinuo Tao, Erdenebayar Battsengel, Tsz Hei Nevin Tsui

A multi-process text analysis tool built in C using `fork()` and `pipe()`. The program
spawns a ring of worker processes, distributes analysis tasks across them using a
token-ring protocol, and prints a formatted summary table of results.

---

## Features

| Statistic | Description |
|---|---|
| Word count | Total number of words in the file |
| Character count | Total number of characters |
| Sentence count | Number of sentences (delimited by `.` `!` `?`) |
| Section count | Number of sections (delimited by blank lines) |
| Longest word | The longest word found in the file |
| Longest sentence | The longest sentence found in the file |
| Average word length | Mean number of characters per word |
| Average sentence length | Mean number of words per sentence |

### How it works

- The parent process spawns **N worker nodes** arranged in a logical ring
- Each node specialises in one analysis task
- Tasks travel clockwise around the ring as `RingMessage` structs written over pipes
- A node forwards any message not addressed to it and processes the one that is
- Results are reported back to the parent via dedicated status pipes
- Multiple nodes run concurrently — no node waits for another before starting work
- A shutdown message travels the ring after all results are collected, cleanly terminating every node

---

## File structure

```
a3/
├── ring.h       — shared message struct, enums, and function prototypes
├── ring.c       — message helper functions (make_token_msg, make_data_msg, etc.)
├── main.c       — pipe setup, forking, and program entry point
├── node.c       — node loop: token forwarding, task processing, shutdown handling
├── jobs.c       — analysis job implementations (word count, sentence length, etc.)
├── parent.c     — task injection, result collection, summary table output
└── Makefile     — build rules
```

---

## Build instructions

Requires only a C compiler (`gcc`) and `make`. No external libraries needed.

```bash
cd a3
make
```

This produces an executable named `a3`.

To clean build artefacts:

```bash
make clean
```

---

## Usage

Pass one or more `.txt` file paths as command line arguments:

```bash
./a3 path/to/file1.txt path/to/file2.txt
```

### Input format

- Files must be plain text (`.txt`)
- Sentences should end with `.`, `!`, or `?`
- Sections are separated by blank lines

### Example output

```
===== RESULTS SUMMARY =====
Task                   file1.txt        file2.txt
----                   ----             ----
Word Count             342              517
Character Count        1893             2740
Sentence Count         18               24
Section Count          4                6
Longest Word           "extraordinary"  "fundamental"
Longest Sentence       "The quick ..."  "In this case ..."
Avg Word Length        5.53             5.30
Avg Sentence Length    19.0             21.5

ACK: 16/16 tasks completed successfully.
```

### Error handling

If a file cannot be opened or does not exist, the corresponding row in the summary
table will show `FAILED` and the program will continue processing any remaining files
and tasks normally.

---

## Platform

Tested on `teach.cs.toronto.edu` (Ubuntu, GCC). No downloaded libraries required.
