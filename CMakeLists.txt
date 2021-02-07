project(atm_controller)

cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_STANDARD 14)

find_package(glog REQUIRED)

file(GLOB SOURCES LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} glog::glog)

add_definitions(-DTESTS)
if (TESTS)
  enable_testing()
  function(generate_test TEST_FILE TEST_NAME)
    get_filename_component(TEST_DISPLAY_NAME ${TEST_NAME} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    target_include_directories(${TEST_NAME} PRIVATE
      ${CMAKE_SOURCE_DIR}/test/include
      ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME} gtest gmock)
    add_custom_command(
      TARGET ${TEST_NAME}
      COMMENT "Running unit tests for ${TEST_DISPLAY_NAME}"
      POST_BUILD
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMAND ${TEST_NAME} --output-on-failures
      )
  endfunction()
  generate_test(${CMAKE_SOURCE_DIR}/test/atm_controller_test.cpp atm.test)
endif(TESTS)
