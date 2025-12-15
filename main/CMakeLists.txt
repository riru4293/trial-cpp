# Use C++23 standard and prefer strict ISO behaviour (no compiler extensions)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
idf_component_register(SRCS "hello_world_main.cpp"
                       PRIV_REQUIRES spi_flash machine
                       INCLUDE_DIRS ".")
