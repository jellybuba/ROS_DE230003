cmake_minimum_required(VERSION 3.5)
project(joint_state_controller)
find_package(ament_cmake REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(rclcpp REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(controller_interface REQUIRED)
find_package(sensor_msgs REQUIRED)

# Load catkin and all dependencies required for this package

# Declare catkin package

include_directories(include ${Boost_INCLUDE_DIR})
add_library(${PROJECT_NAME} src/joint_state_controller.cpp)
ament_target_dependencies(${PROJECT_NAME} "pluginlib" "controller_interface" "realtime_tools" "sensor_msgs" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(realtime_tools rclcpp hardware_interface pluginlib controller_interface sensor_msgs)
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

install(FILES joint_state_plugin.xml
              joint_state_controller.yaml
              joint_state_controller.launch
  DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(BUILD_TESTING)
  find_package(rostest REQUIRED)
  target_link_libraries(joint_state_controller_test ${PROJECT_NAME})

  add_rostest_gtest(joint_state_controller_test
                    test/joint_state_controller.test
                    test/joint_state_controller_test.cpp)
endif()
