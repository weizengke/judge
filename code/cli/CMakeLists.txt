file(GLOB_RECURSE all_files
*.cpp
*.cc
*.c++
*.c
*.C
)

add_library(cli STATIC ${all_files})

