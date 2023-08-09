cmake_minimum_required(VERSION 3.5.0)
project(cardiograf-database 
        LANGUAGES C CXX 
        VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)

include_directories(${HDF5_INCLUDE_DIRS})
# include_directories(${CMAKE_SOURCE_DIR}/_DuckDB/)
# include(CTest)
# enable_testing()

# HDF5
find_package(h5cpp 0.5.2)
find_package(hdf5 CONFIG 1.10.4)
# Arrow Parquet
find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(ArrowDataset REQUIRED)
# json для разметки
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(cardiograf-database main.cpp)

find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(cardiograf-database ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries(cardiograf-database ${Boost_SYSTEM_LIBRARY})
target_link_libraries(cardiograf-database h5cpp hdf5::hdf5) 

target_link_libraries(cardiograf-database Arrow::arrow_shared)
target_link_libraries(cardiograf-database Parquet::parquet_shared)
target_link_libraries(cardiograf-database ArrowDataset::arrow_dataset_shared)
# DuckDB
target_link_libraries(cardiograf-database ${CMAKE_SOURCE_DIR}/_libs/_DuckDB/libduckdb.so)

target_link_libraries(cardiograf-database nlohmann_json::nlohmann_json)

