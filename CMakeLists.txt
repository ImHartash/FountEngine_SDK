cmake_minimum_required(VERSION 3.16)
project(FountEngineSDK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(FountModelConverter)
add_subdirectory(FountEngineTextureConverter)
add_subdirectory(FountEngineMaterialBacker)
add_subdirectory(FountEnginePacker)