# Poker AI with Deep CFR

A poker AI implementation using Deep Counterfactual Regret Minimization (Deep CFR) with a custom neural network library.

## Goals

- Implement a poker engine
- Implement a poker AI using Deep CFR

I want to make this as modular as possible so that I can experiment with different architectures and algorithms, then test them in the poker engine against each other.

I also wanted to learn more about poker theory and deep learning in general, so that's the main motivation for this project. 


## Project Structure

- `main.cpp`: Main file for running the poker AI.
- `engine/`: Directory for the poker engine implementation.
- `ai/deep_cfr/`: Directory for the Deep CFR implementation.
- `lib/`: Directory for the custom neural network library.


### TODO

- [ ] Fix bug where all players go all in at the same time
- [ ] Make general game interface for AI to use
- [ ] Make an engine to play against the AI
- [ ] Adjust game engine to be thread safe

# Contributing

to start
`git clone https://github.com/dylan-almonte/poker-ai`

Make a branch to start developing
`git checkout -b <branchname>`
