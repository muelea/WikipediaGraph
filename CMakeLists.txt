cmake_minimum_required(VERSION 2.8)

# compile flags for everything
set(CMAKE_CXX_FLAGS "-pedantic -ansi -std=c++11 -Wall -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")
find_package(Eigen3 REQUIRED)
find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

find_package(Boost REQUIRED COMPONENTS regex)

enable_testing()
find_package(GTest REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS})

add_executable(main main.cpp wikiarticle.cpp parse_wiki_xml.cpp shortest_path.cpp bag_of_words.cpp)
set_target_properties(main PROPERTIES COMPILE_FLAGS "-xhost")

add_executable(main.mic main.cpp wikiarticle.cpp parse_wiki_xml.cpp shortest_path.cpp bag_of_words.cpp)
set_target_properties(main.mic PROPERTIES COMPILE_FLAGS "-mmic -DIS_MIC" LINK_FLAGS "-mmic")

add_executable(TEST testing.cpp wikiarticle.cpp parse_wiki_xml.cpp shortest_path.cpp bag_of_words.cpp)
set_target_properties(TEST PROPERTIES COMPILE_FLAGS "-xhost")

target_link_libraries(main ${Boost_LIBRARIES})
target_link_libraries(TEST ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES})

add_test(TEST TEST)

#add_executable(TEST.mic testing.cpp wikiarticle.cpp parse_wiki_xml.cpp shortest_path.cpp)
#set_target_properties(TEST.mic PROPERTIES COMPILE_FLAGS "-mmic" LINK_FLAGS "-mmic")
#target_link_libraries(TEST.mic ${GTEST_BOTH_LIBRARIES})
#add_test(TEST TEST.mic)

# compile using CC=mpiicc CXX=mpiicpc cmake ..
# 28.01. compile using CC=mpicc CXX=mpicxx cmake ..

# run using the following line
# mpirun -machinefile <machine_file> <executable>