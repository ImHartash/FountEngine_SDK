include(FetchContent)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

add_executable(fnttex main.cpp)
target_include_directories(fnttex PRIVATE ${stb_SOURCE_DIR})