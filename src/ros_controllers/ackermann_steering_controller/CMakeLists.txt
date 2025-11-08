cmake_minimum_required(VERSION 3.5)
project(ackermann_steering_controller)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(diff_drive_controller REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(urdf REQUIRED)

include_directories(
  include
  ${Boost_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} src/ackermann_steering_controller.cpp src/odometry.cpp)
ament_target_dependencies(${PROJECT_NAME} ${Boost_LIBRARIES} "rclcpp" "controller_manager" "geometry_msgs" "nav_msgs" "std_msgs" "std_srvs" "pluginlib" "tf2_ros" "diff_drive_controller" "controller_interface" "controller_manager_msgs" "realtime_tools" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(controller_interface diff_drive_controller hardware_interface nav_msgs realtime_tools rclcpp tf2_ros)
ament_package()

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(FILES ackermann_steering_controller_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if (BUILD_TESTING)
  find_package(catkin REQUIRED COMPONENTS
    controller_manager
    geometry_msgs
    rostest
    rosunit
    std_msgs
    std_srvs
    tf
    xacro
  )
  include_directories(${catkin_INCLUDE_DIRS})

  add_executable(${PROJECT_NAME}_ackermann_steering_bot test/common/src/ackermann_steering_bot.cpp)

  target_link_libraries(${PROJECT_NAME}_ackermann_steering_bot ${catkin_LIBRARIES})
  add_dependencies(tests ${PROJECT_NAME}_ackermann_steering_bot)
  target_link_libraries(${PROJECT_NAME}_test ${catkin_LIBRARIES})
  target_link_libraries(${PROJECT_NAME}_nan_test ${catkin_LIBRARIES})
  target_link_libraries(${PROJECT_NAME}_limits_test ${catkin_LIBRARIES})
  target_link_libraries(${PROJECT_NAME}_timeout_test ${catkin_LIBRARIES})
  target_link_libraries(ackermann_steering_controller_fail_test ${catkin_LIBRARIES})
  target_link_libraries(${PROJECT_NAME}_odom_tf_test ${catkin_LIBRARIES})
  target_link_libraries(${PROJECT_NAME}_default_odom_frame_test ${catkin_LIBRARIES})
  target_link_libraries(ackermann_steering_controller_odom_frame_test ${catkin_LIBRARIES})

  add_rostest(test/ackermann_steering_controller_open_loop_test/ackermann_steering_controller_open_loop.test)
  add_rostest(test/ackermann_steering_controller_no_wheel_test/ackermann_steering_controller_no_wheel.test)
  add_rostest(test/ackermann_steering_controller_no_steer_test/ackermann_steering_controller_no_steer.test)
  add_rostest(test/ackermann_steering_controller_radius_param_test/ackermann_steering_controller_radius_param.test)
  add_rostest(test/ackermann_steering_controller_radius_param_fail_test/ackermann_steering_controller_radius_param_fail.test)
  add_rostest(test/ackermann_steering_controller_separation_param_test/ackermann_steering_controller_separation_param.test)
  add_rostest_gtest(${PROJECT_NAME}_test
    test/ackermann_steering_controller_test/ackermann_steering_controller.test
    test/ackermann_steering_controller_test/ackermann_steering_controller_test.cpp)

  add_rostest_gtest(${PROJECT_NAME}_nan_test
    test/ackermann_steering_controller_nan_test/ackermann_steering_controller_nan.test
    test/ackermann_steering_controller_nan_test/ackermann_steering_controller_nan_test.cpp)

  add_rostest_gtest(${PROJECT_NAME}_limits_test
    test/ackermann_steering_controller_limits_test/ackermann_steering_controller_limits.test
    test/ackermann_steering_controller_limits_test/ackermann_steering_controller_limits_test.cpp)

  add_rostest_gtest(${PROJECT_NAME}_timeout_test
    test/ackermann_steering_controller_timeout_test/ackermann_steering_controller_timeout.test
    test/ackermann_steering_controller_timeout_test/ackermann_steering_controller_timeout_test.cpp)

  add_rostest_gtest(ackermann_steering_controller_fail_test
    test/ackermann_steering_controller_fail_test/ackermann_steering_controller_wrong.test
    test/ackermann_steering_controller_fail_test/ackermann_steering_controller_fail_test.cpp)

  add_rostest_gtest(${PROJECT_NAME}_odom_tf_test
    test/ackermann_steering_controller_odom_tf_test/ackermann_steering_controller_odom_tf.test
    test/ackermann_steering_controller_odom_tf_test/ackermann_steering_controller_odom_tf_test.cpp)

  add_rostest_gtest(${PROJECT_NAME}_default_odom_frame_test
    test/ackermann_steering_controller_default_odom_frame_test/ackermann_steering_controller_default_odom_frame.test
    test/ackermann_steering_controller_default_odom_frame_test/ackermann_steering_controller_default_odom_frame_test.cpp)

  add_rostest_gtest(ackermann_steering_controller_odom_frame_test
    test/ackermann_steering_controller_odom_frame_test/ackermann_steering_controller_odom_frame.test
    test/ackermann_steering_controller_odom_frame_test/ackermann_steering_controller_odom_frame_test.cpp)
endif()
