cmake_minimum_required(VERSION 3.5)
project(gripper_action_controller)
find_package(ament_cmake REQUIRED)
find_package(actionlib REQUIRED)
find_package(angles REQUIRED)
find_package(cmake_modules REQUIRED)
find_package(controller_interface REQUIRED)
find_package(controller_manager REQUIRED)
find_package(control_msgs REQUIRED)
find_package(control_toolbox REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(rclcpp REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(urdf REQUIRED)
find_package(xacro REQUIRED)
find_package(pluginlib REQUIRED)

include_directories(
  include
)

add_library(gripper_action_controller
   src/gripper_action_controller.cpp
   include/gripper_action_controller/gripper_action_controller.h
)

ament_target_dependencies(gripper_action_controller
 "pluginlib"
 "control_msgs"
 "control_toolbox"
 "controller_interface"
 "realtime_tools"
 "urdf"
 "hardware_interface"
 "actionlib"
)
ament_export_include_directories(include)
ament_export_libraries(gripper_action_controller)
ament_export_dependencies(actionlib cmake_modules controller_interface control_msgs hardware_interface realtime_tools trajectory_msgs)
ament_package()

# Install library
install(TARGETS gripper_action_controller
   ARCHIVE DESTINATION lib
   LIBRARY DESTINATION lib
   RUNTIME DESTINATION bin
)

install(DIRECTORY include/${PROJECT_NAME}/
   DESTINATION include/${PROJECT_NAME}
)

install(FILES ros_control_plugins.xml
   DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
