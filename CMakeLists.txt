CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mylisp)

SET(CMAKE_C_FLAGS "-Wall -O0 -g3 -pg")

ADD_EXECUTABLE(mylisp
    src/main.c
    src/treeoperation.c
    src/parser.c
    src/variable.c
    src/function.c
    src/eval.c
    src/stack.c
)

