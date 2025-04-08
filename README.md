# Poker AI with Deep CFR

A poker AI implementation using Deep Counterfactual Regret Minimization (Deep CFR) with a custom neural network library.

## Goals

- Implement a poker engine
- Implement a poker AI using Deep CFR

I want to make this as modular as possible so that I can experiment with different architectures and algorithms, then test them in the poker engine against each other.

I also wanted to learn more about poker theory and deep learning in general, so that's the main motivation for this project. 


## Project Structure

- `main.cpp`: Main file for running the poker AI.
- `engine_v2/`: Directory for the poker engine implementation.
- `ai/deep_cfr/`: Directory for the Deep CFR implementation.
- `lib/`: Directory for the custom neural network library.


### TODO

- [X] Fix bug where all players go all in at the same time
- [ ] Make general game interface for AI to use
- [ ] Make an engine to play against the AI
- [ ] Adjust game engine to be thread safe

# Contributing

to start
`git clone https://github.com/dylan-almonte/poker-ai`

Make a branch to start developing
`git checkout -b <branchname>`

## Instructions for Andy 

1. to download the repo
`git clone https://github.com/dylan-almonte/poker-ai`

2. change to the engine branch
`git checkout engine`

3. create a new branch with your username/ and what your working on (this will branch off of engine)
`git checkout -b <username>/<feature>`
like this
`git checkout -b dcabahug/engine`

5. push your branch to the remote repo
`git push -u origin <branch name>`
this is the same branch name you made in prev step
example:
`git push -u origin dcabahug/engine`

6. you can start working now

to build your code, ask chat to make the run.sh files into .bat files

