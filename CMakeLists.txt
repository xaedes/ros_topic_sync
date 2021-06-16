cmake_minimum_required(VERSION 2.8.3)
project(ros_topic_sync)

set(CMAKE_CXX_STANDARD 11)

# catkin packages
find_package(
    catkin 
    REQUIRED COMPONENTS 
    ros_cmake_generate_dependency
    roscpp 
    rospy 
    message_filters
)

# other packages
find_package(Boost REQUIRED COMPONENTS signals thread)

# create catkin package for project
set(LIBRARY_NAME "${PROJECT_NAME}")
catkin_python_setup()
catkin_package(
    LIBRARIES ${LIBRARY_NAME}
    INCLUDE_DIRS include
    CATKIN_DEPENDS 
    ros_cmake_generate_dependency
    roscpp 
    rospy 
    message_filters
    DEPENDS
    Boost
)  

## Create the library
add_library(
    ${LIBRARY_NAME} 
    src/empty.cpp
)

include(GenerateDependency)
generate_dependency(${LIBRARY_NAME} catkin)
generate_dependency(${LIBRARY_NAME} Boost)
target_include_directories(${LIBRARY_NAME} PUBLIC include/)

add_dependencies(${LIBRARY_NAME} ${catkin_EXPORTED_TARGETS} ${${PROJECT_NAME}_EXPORTED_TARGETS})

## Mark the library for installations
install(TARGETS ${LIBRARY_NAME}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

# install header files
install(
    DIRECTORY 
    include/${PROJECT_NAME}/
    DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
    FILES_MATCHING PATTERN "*.h"
)

