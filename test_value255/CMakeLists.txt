cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# テスト専用のコンポーネント構成
set(EXTRA_COMPONENT_DIRS "../components/value/test" "../components")

project(test_value255)
