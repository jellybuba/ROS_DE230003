cmake_minimum_required(VERSION 3.5)
project(velocity_controllers)
find_package(ament_cmake REQUIRED)
find_package(angles REQUIRED)
find_package(control_msgs REQUIRED)
find_package(control_toolbox REQUIRED)
find_package(controller_interface REQUIRED)
find_package(forward_command_controller REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(urdf REQUIRED)
find_package(pluginlib REQUIRED)

# Load catkin and all dependencies required for this package

# Declare catkin package

include_directories(include ${Boost_INCLUDE_DIR})

add_library(${PROJECT_NAME}
  src/joint_velocity_controller.cpp
  src/joint_position_controller.cpp
  src/joint_group_velocity_controller.cpp
)

ament_target_dependencies(${PROJECT_NAME} "realtime_tools" "std_msgs" "forward_command_controller" "pluginlib" "control_msgs" "control_toolbox" "controller_interface" "angles" "urdf" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(angles control_msgs control_toolbox controller_interface forward_command_controller realtime_tools urdf)
ament_package()

# Install
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

# Install library
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(FILES velocity_controllers_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
