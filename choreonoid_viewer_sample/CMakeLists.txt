cmake_minimum_required(VERSION 3.0.2)
project(choreonoid_viewer_sample)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

find_package(catkin REQUIRED COMPONENTS
  choreonoid_viewer)

find_package(choreonoid REQUIRED)

catkin_package(
)

###########
## Build ##
###########

include_directories(
 include
 ${catkin_INCLUDE_DIRS}
 ${CHOREONOID_INCLUDE_DIRS}
)

add_library(choreonoid_viewer_sample_lib
  src/choreonoid_viewer_sample_lib.cpp
)

target_link_libraries(choreonoid_viewer_sample_lib
  ${catkin_LIBRARIES}
  ${CHOREONOID_BODY_LIBRARIES}
  )

add_executable(choreonoid_viewer_sample
  src/choreonoid_viewer_sample.cpp
  )
target_link_libraries(choreonoid_viewer_sample
  ${catkin_LIBRARIES}
  ${CHOREONOID_BODY_LIBRARIES}
  choreonoid_viewer_sample_lib
  )

add_library(CnoidViewerSamplePlugin SHARED
  src/CnoidViewerSamplePlugin.cpp
  )
target_link_libraries(CnoidViewerSamplePlugin
  ${catkin_LIBRARIES}
  ${CHOREONOID_BODY_PLUGIN_LIBRARIES}
  choreonoid_viewer_sample_lib
  )
set_target_properties(CnoidViewerSamplePlugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CHOREONOID_PLUGIN_DIR})

#############
## Testing ##
#############

#add_subdirectory(sample)
