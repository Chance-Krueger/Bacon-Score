# Bacon-Score

## Description
A C program that calculates the “Bacon score” of actors using breadth-first search on a graph built from movie/actor data. It inspired our Oracle Bacon Score version by introducing the graph-based connection concept, flexible CLI design, and shortest path tracing to measure actor relationships via shared film appearances.

## What I learned: 
    - Graph theory fundamentals and representing relationships with vertices and edges.
    - Implementing breadth-first search to find shortest paths.
    - Parsing input files and handling command-line arguments.
    - Using linked lists and queues for graph representation and traversal.
    - Tracking paths for detailed output.
    - Managing error handling and edge cases.
    - Writing clean, modular, and efficient code.

## How To run it:
### Compile the program using a C compiler, for example:
    - gcc BaconScore.c -o BaconScore
### Run the executable from the command line:
    - ./BaconScore inputFile
    - inputFile is the text file with movies and actors.

### Once running
    - type an actor’s name and press Enter to get their Bacon score.
    - Keep entering actor names until you want to stop (Ctrl+D on Unix, Ctrl+Z on Windows).

## Example usage:
    - ./BaconScore -l movies.txt
    - Then input actor names interactively.

## Future Improvements
    - make a -l option so that -> -l is an optional flag to also print the full connection path (not just the score).
        - can do -l, but wont work currently (print the full connection path).
