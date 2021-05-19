file(GLOB_RECURSE all_files
*.cpp
*.cc
*.c++
*.c
*.C
)

add_library(main ${all_files})

