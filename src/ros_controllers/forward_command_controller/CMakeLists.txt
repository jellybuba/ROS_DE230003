cmake_minimum_required(VERSION 3.5)
project(forward_command_controller)
find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(std_msgs REQUIRED)
find_package(realtime_tools REQUIRED)

# Load catkin and all dependencies required for this package

# Declare catkin package

ament_export_include_directories(include)
ament_export_dependencies(controller_interface hardware_interface std_msgs realtime_tools)
ament_package()

# Install
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
