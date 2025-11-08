cmake_minimum_required(VERSION 3.5)
project(four_wheel_steering_controller)
find_package(ament_cmake REQUIRED)
find_package(${${PROJECT_NAME}_CATKIN_DEPS} REQUIRED)

set(${PROJECT_NAME}_CATKIN_DEPS
    controller_interface
    nav_msgs
    four_wheel_steering_msgs
    realtime_tools
    tf
    urdf_geometry_parser
    pluginlib)

include_directories(
  include
)

add_library(${PROJECT_NAME} src/four_wheel_steering_controller.cpp src/odometry.cpp src/speed_limiter.cpp)
ament_target_dependencies(${PROJECT_NAME} "four_wheel_steering_msgs" "urdf_geometry_parser" "rclcpp" "geometry_msgs" "nav_msgs" "controller_manager" "std_msgs" "std_srvs" "pluginlib" "tf2_ros" "controller_interface" "realtime_tools" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(${${PROJECT_NAME}_CATKIN_DEPS})
ament_package()

# Install library
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  
)

install(FILES four_wheel_steering_controller_plugins.xml
    DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(BUILD_TESTING)
  find_package(catkin REQUIRED COMPONENTS rosgraph_msgs rostest std_srvs controller_manager tf)
  include_directories(${catkin_INCLUDE_DIRS})

  add_executable(four_wheel_steering test/src/four_wheel_steering.cpp)
  target_link_libraries(four_wheel_steering ${catkin_LIBRARIES})

  add_dependencies(tests four_wheel_steering)
  target_link_libraries(four_wheel_steering_controller_twist_cmd_test ${catkin_LIBRARIES})
  target_link_libraries(four_wheel_steering_controller_4ws_cmd_test ${catkin_LIBRARIES})
  target_link_libraries(four_wheel_steering_wrong_config_test ${catkin_LIBRARIES})

  add_rostest_gtest(four_wheel_steering_controller_twist_cmd_test
                    test/four_wheel_steering_controller_twist_cmd.test
                    test/src/four_wheel_steering_twist_cmd_test.cpp)

  add_rostest_gtest(four_wheel_steering_controller_4ws_cmd_test
                    test/four_wheel_steering_controller_4ws_cmd.test
                    test/src/four_wheel_steering_4ws_cmd_test.cpp)

  add_rostest_gtest(four_wheel_steering_wrong_config_test
                    test/four_wheel_steering_wrong_config.test
                    test/src/four_wheel_steering_wrong_config.cpp)

endif()
