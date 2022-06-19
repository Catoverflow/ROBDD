# ROBDD

**An ROBDD constructor and SAT/unSAT solver (WIP)** written in C++

![example](README.assets/example.svg)

## Requirement

You need to install flex, bison, gcc for compiling and graphviz for image generating.

## How-to

Compile first

`make ROBDD`

Open `ROBDD` binary, type in any binary function you like, and press ENTER. It will generate `res.dot`

`make image` Process `res.dot` by Graphviz and generate `res.svg` as ROBDD output.

## Credits

Henrik Reif Andersen, who wrote *[An Introduction to Binary Decision Diagrams](https://www.cs.utexas.edu/~isil/cs389L/bdd.pdf)* and it's my major reference.