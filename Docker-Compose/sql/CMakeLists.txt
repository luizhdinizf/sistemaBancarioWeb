cmake_minimum_required(VERSION 3.10)

project(CppMysqlWeb)
SET(GCC_COVERAGE_LINK_FLAGS    "-lmysqlcppconn")


include_directories(include /usr/local/include/mysql)
include_directories(include /usr/local/include/mysql++)
#include_directories(${root_SOURCE_DIR}/date-Howard)
#add_subdirectory(date-Howard)
#add_subdirectory(sqlpp11)

