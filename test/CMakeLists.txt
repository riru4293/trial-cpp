cmake_minimum_required(VERSION 3.22)

# 親プロジェクトのcomponentsディレクトリをインクルード
set(EXTRA_COMPONENT_DIRS "../components")

# テストするコンポーネントを指定
set(TEST_COMPONENTS "value" CACHE STRING "List of components to test")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(trial-cpp-test)
