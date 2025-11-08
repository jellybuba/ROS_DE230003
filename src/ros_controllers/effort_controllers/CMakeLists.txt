cmake_minimum_required(VERSION 3.5)
project(effort_controllers)
find_package(ament_cmake REQUIRED)
find_package(angles REQUIRED)
find_package(controller_interface REQUIRED)
find_package(control_msgs REQUIRED)
find_package(control_toolbox REQUIRED)
find_package(forward_command_controller REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(urdf REQUIRED)
find_package(pluginlib REQUIRED)

# Load catkin and all dependencies required for this package

include_directories(include ${Boost_INCLUDE_DIR})

# Declare catkin package

add_library(${PROJECT_NAME}
  src/joint_effort_controller.cpp
  src/joint_velocity_controller.cpp
  src/joint_position_controller.cpp
  src/joint_group_effort_controller.cpp
  src/joint_group_position_controller.cpp
)

ament_target_dependencies(${PROJECT_NAME} "realtime_tools" "std_msgs" "forward_command_controller" "pluginlib" "control_msgs" "control_toolbox" "controller_interface" "angles" "urdf" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(angles controller_interface control_msgs control_toolbox forward_command_controller realtime_tools urdf)
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

install(FILES effort_controllers_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(BUILD_TESTING)
  find_package(rostest REQUIRED)
  find_package(controller_manager REQUIRED)
  find_package(controller_manager_msgs REQUIRED)
  find_package(rosgraph_msgs REQUIRED)
  find_package(sensor_msgs REQUIRED)
  find_package(std_msgs REQUIRED)

  include_directories(
    ${controller_manager_INCLUDE_DIRS}
    ${hardware_interface_INCLUDE_DIRS}
    ${rosgraph_msgs_INCLUDE_DIRS}
    ${sensor_msgs_INCLUDE_DIRS}
    ${std_msgs_INCLUDE_DIRS}
  )

  add_executable(simple_bot_hw test/simple_bot_hw.cpp)
  target_link_libraries(simple_bot_hw
    ${catkin_LIBRARIES}
    ${controller_manager_LIBRARIES}
    ${hardware_interface_LIBRARIES}
  )
  add_dependencies(tests simple_bot_hw)
  target_link_libraries(
    effort_position_controller_test
    ${catkin_LIBRARIES}
  )

  add_rostest_gtest(effort_position_controller_test
    test/effort_position_controller.test
    test/simple_bot_goto_position.cpp
  )
endif()
