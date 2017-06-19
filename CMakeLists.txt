cmake_minimum_required(VERSION 2.4)
project(pix2image)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
add_definitions(-std=c++11)
find_package( OpenCV REQUIRED)

find_package( Boost 1.40 COMPONENTS log log_setup thread
  program_options REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)
#add_executable( pix2image pix2image.cpp)
#target_link_libraries( pix2image ${OpenCV_LIBS} ) 

add_executable(pix2image pix2image.cpp)
target_link_libraries( pix2image  
  -lpthread
  ${OpenCV_LIBS}
  ${Boost_LIBRARIES})
