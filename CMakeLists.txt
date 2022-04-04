# root level cmakelists.txt for deepsea project 

cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(game)

# To make a debug build
set(CMAKE_CXX_FLAGS "-g")

find_package(OpenMP REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem thread)



set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#target_link_libraries(my_app PRIVATE Threads::Threads)


#add_definitions(-DTRACY_ENABLE)

# Define the framework files.
set(SRCS
	#tracy/Tracy.hpp
	#tracy/TracyClient.cpp
	src/game.cpp
)

link_directories(
	${Boost_LIBRARY_DIRS}
)

include_directories (
	${SDL2_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	"tracy"
)

add_executable(game ${SRCS} )

target_link_libraries (
	game
	OpenMP::OpenMP_CXX
	${SDL2_LIBRARIES}
	${OPENGL_LIBRARIES}
	${Boost_LIBRARIES}
	${OPENSSL_LIBRARIES} dl
	Threads::Threads
)


