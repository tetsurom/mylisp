CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mylisp)


SET(CMAKE_C_FLAGS_DEBUG "-Wall -O0 -g3 -pg")
SET(CMAKE_C_FLAGS_RELEASE "-Wall -O2 -g -DNDEBUG")

ADD_EXECUTABLE(mylisp
    src/main.c
    src/treeoperation.c
    src/parser.c
    src/variable.c
    src/function.c
    src/eval.c
    src/stack.c
    src/istack.c
    src/lisp.c
    src/compiler.c
)

include_directories("include")

