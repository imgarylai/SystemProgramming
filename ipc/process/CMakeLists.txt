cmake_minimum_required(VERSION 3.8)
project(sp)

set(CMAKE_C_STANDARD 99)

#set(SOURCE_FILES uniqify.c)
add_executable(exec exec.c)
add_executable(child-ls child-ls.c)
add_executable(faster-pipeline-shell faster-pipeline-shell.c)
add_executable(fork fork.c)
add_executable(pipe pipe.c)
add_executable(pipe-example pipe-example.c)
add_executable(pipe-example-2 pipe-example-2.c)
add_executable(random-gen random-gen.c)
add_executable(red red.c)
add_executable(simple-pipeline-shell simple-pipeline-shell.c)
add_executable(todo-pipeline-shell todo-pipeline-shell.c)
add_executable(ttt ttt.c)
add_executable(u2 u2.c)
add_executable(a a.c)

set(SOURCE_FILES uniqify.c)
add_executable(uniqify ${SOURCE_FILES})