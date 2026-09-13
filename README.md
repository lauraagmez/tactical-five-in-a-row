# Tactical Five-in-a-Row

**Development period:** May – June 2026  
**Course:** Artificial Intelligence  
**University:** University of Granada  
**Language:** C++

Adversarial-search Artificial Intelligence project developed in C++ for a tactical variant of Five-in-a-Row played on a 9×9 board.

The project focuses on the design of an intelligent game-playing agent capable of analysing possible future states, evaluating board positions and selecting competitive moves against different opponents.

## Overview

Tactical Five-in-a-Row extends the traditional alignment game with a larger search space and additional strategic rules.

The objective is to align five pieces horizontally, vertically or diagonally while dealing with special board mechanics and asymmetric turns.

The intelligent agent uses adversarial search techniques to choose its moves by exploring possible future game states.

## Artificial Intelligence techniques

### Minimax

The Minimax algorithm explores the game tree assuming that both players make optimal decisions.

The agent evaluates possible future board states and selects the move that maximises its expected result while considering the opponent's best possible response.

### Alpha-Beta pruning

Alpha-Beta pruning is used to improve the efficiency of Minimax by avoiding branches of the game tree that cannot affect the final decision.

This allows the agent to explore deeper positions while reducing the number of evaluated states.

### Heuristic evaluation

Because exhaustively exploring a 9×9 game tree is computationally impractical, the agent uses heuristic evaluation functions to estimate the quality of non-terminal board states.

The heuristic is designed to guide the search towards strategically advantageous positions and improve performance against competitive opponents.

### Status search

A complete search mode is also implemented for smaller game configurations, allowing the theoretical result of a position to be determined as:

- Victory
- Draw
- Defeat

## Competition mode

The competition version uses a **9×9 board** with the objective of connecting **five pieces**.

It introduces additional rules including:

- Asymmetric turn sequence
- Placement restrictions based on board phases
- Mandatory adjacency to existing pieces
- Special board cells with different effects

These mechanics significantly increase the size and complexity of the search space.

## Project structure

```text
tactical-five-in-a-row/
├── AgenteEstudiante.cpp
├── AgenteEstudiante.hpp
└── README.md
