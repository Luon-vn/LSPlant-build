cmake_minimum_required(VERSION 3.22)
macro(SET_OPTION option value)
    set(${option} ${value} CACHE INTERNAL "" FORCE)
endmacro()

SET_OPTION(LSPLT_STANDALONE ON)

project(nativelib)
# Main source
# set(ANDROID_STL "c++_shared") # CXX STL mode for lib
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
# SET_OPTION(Plugin.Android.BionicLinkerUtil ON)
add_library(${CMAKE_PROJECT_NAME} SHARED ${CMAKE_SOURCE_DIR}/main.cpp ${CMAKE_SOURCE_DIR}/elf_symbol_resolver.cpp ${CMAKE_SOURCE_DIR}/elf_util.cc)
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE c_std_23 cxx_std_23)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE log )

# lsplt
include_directories(${CMAKE_SOURCE_DIR}/lsplt/include)
add_subdirectory(${CMAKE_SOURCE_DIR}/lsplt)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE lsplt_static)

if (LSPLT_STANDALONE)
    find_package(cxx REQUIRED CONFIG)
    link_libraries(cxx::cxx)
else()
  # ./libcxx prebuilt
  include_directories(${CMAKE_SOURCE_DIR}/libcxx/include)
  link_libraries(${CMAKE_SOURCE_DIR}/libcxx/${CMAKE_ANDROID_ARCH_ABI}.a)
endif()
