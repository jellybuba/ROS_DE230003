cmake_minimum_required(VERSION 3.5)
project(diff_drive_controller)
find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(control_msgs REQUIRED)
find_package(dynamic_reconfigure REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(urdf REQUIRED)
find_package(pluginlib REQUIRED)

generate_dynamic_reconfigure_options(cfg/DiffDriveController.cfg)

include_directories(
  include
)

add_library(${PROJECT_NAME} src/diff_drive_controller.cpp src/odometry.cpp src/speed_limiter.cpp)
ament_target_dependencies(${PROJECT_NAME} "realtime_tools" "geometry_msgs" "nav_msgs" "dynamic_reconfigure" "control_msgs" "pluginlib" "tf2_ros" "controller_interface" "urdf" "hardware_interface")

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package()

install(DIRECTORY include/${PROJECT_NAME}/
        DESTINATION include/${PROJECT_NAME}
)

# Install library
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  
)

install(FILES diff_drive_controller_plugins.xml
    DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if (BUILD_TESTING)
  find_package(catkin REQUIRED COMPONENTS controller_manager rosgraph_msgs rostest std_srvs tf)
  include_directories(include ${catkin_INCLUDE_DIRS})

  add_executable(diffbot test/diffbot.cpp)
  target_link_libraries(diffbot ${catkin_LIBRARIES})

  add_executable(skidsteerbot test/skidsteerbot.cpp)
  target_link_libraries(skidsteerbot ${catkin_LIBRARIES})

  add_dependencies(tests diffbot skidsteerbot)
  target_link_libraries(diff_drive_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_nan_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_limits_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_timeout_test ${catkin_LIBRARIES})
  add_rostest(test/diff_drive_multipliers.test)
  add_rostest(test/diff_drive_left_right_multipliers.test)
  target_link_libraries(diff_drive_fail_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_odom_tf_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_default_cmd_vel_out_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_pub_cmd_vel_out_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_default_wheel_joint_controller_state_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_publish_wheel_joint_controller_state_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_default_odom_frame_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_odom_frame_test ${catkin_LIBRARIES})
  target_link_libraries(diff_drive_multiple_cmd_vel_publishers_test ${catkin_LIBRARIES})
  add_rostest(test/diff_drive_open_loop.test)
  add_rostest(test/skid_steer_controller.test)
  add_rostest(test/skid_steer_no_wheels.test)
  add_rostest(test/diff_drive_radius_sphere.test)
  add_rostest(test/diff_drive_radius_param.test)
  add_rostest(test/diff_drive_radius_param_fail.test)
  add_rostest(test/diff_drive_separation_param.test)
  add_rostest(test/diff_drive_bad_urdf.test)
  target_link_libraries(diff_drive_dyn_reconf_test ${catkin_LIBRARIES})

  add_rostest_gtest(diff_drive_test
    test/diff_drive_controller.test
    test/diff_drive_test.cpp)
  add_rostest_gtest(diff_drive_nan_test
    test/diff_drive_controller_nan.test
    test/diff_drive_nan_test.cpp)
  add_rostest_gtest(diff_drive_limits_test
    test/diff_drive_controller_limits.test
    test/diff_drive_limits_test.cpp)
  add_rostest_gtest(diff_drive_timeout_test
    test/diff_drive_timeout.test
    test/diff_drive_timeout_test.cpp)
  add_rostest_gtest(diff_drive_fail_test
    test/diff_drive_wrong.test
    test/diff_drive_fail_test.cpp)
  add_rostest_gtest(diff_drive_odom_tf_test
    test/diff_drive_odom_tf.test
    test/diff_drive_odom_tf_test.cpp)
  add_rostest_gtest(diff_drive_default_cmd_vel_out_test
    test/diff_drive_default_cmd_vel_out.test
    test/diff_drive_default_cmd_vel_out_test.cpp)
  add_rostest_gtest(diff_drive_pub_cmd_vel_out_test
    test/diff_drive_pub_cmd_vel_out.test
    test/diff_drive_pub_cmd_vel_out_test.cpp)
  add_rostest_gtest(diff_drive_default_wheel_joint_controller_state_test
    test/diff_drive_default_wheel_joint_controller_state.test
    test/diff_drive_default_wheel_joint_controller_state_test.cpp)
  add_rostest_gtest(diff_drive_publish_wheel_joint_controller_state_test
    test/diff_drive_publish_wheel_joint_controller_state.test
    test/diff_drive_publish_wheel_joint_controller_state_test.cpp)
  add_rostest_gtest(diff_drive_default_odom_frame_test
    test/diff_drive_default_odom_frame.test
    test/diff_drive_default_odom_frame_test.cpp)
  add_rostest_gtest(diff_drive_odom_frame_test
    test/diff_drive_odom_frame.test
    test/diff_drive_odom_frame_test.cpp)
  add_rostest_gtest(diff_drive_multiple_cmd_vel_publishers_test
    test/diff_drive_multiple_cmd_vel_publishers.test
    test/diff_drive_multiple_cmd_vel_publishers_test.cpp)
  add_rostest_gtest(diff_drive_dyn_reconf_test
    test/diff_drive_dyn_reconf.test
    test/diff_drive_dyn_reconf_test.cpp)
endif()
