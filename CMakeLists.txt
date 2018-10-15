cmake_minimum_required(VERSION 3.8)

if ($ENV{TRAVIS_BUILD_NUMBER})
    project(join_server VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else ()
    project(join_server VERSION 0.0.1)
endif ()

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads)
include_directories(${Boost_INCLUDE_DIR})


#include_directories("${PROJECT_SOURCE_DIR}/include")
add_executable(join_server src/main.cpp)

target_link_libraries(
        join_server
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS join_server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT artbataev@gmail.com)

include(CPack)

#enable_testing()
#add_test(test test_async)
