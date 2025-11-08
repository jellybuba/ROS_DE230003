cmake_minimum_required(VERSION 3.5)
project(combined_robot_hw_tests)
find_package(ament_cmake REQUIRED)
find_package(combined_robot_hw REQUIRED)
find_package(controller_manager REQUIRED)
find_package(controller_manager_msgs REQUIRED)
find_package(controller_manager_tests REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)

# Load catkin and all dependencies required for this package

# Declare a catkin package

ament_package()

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(rostest REQUIRED)

  # Specify header include paths
  include_directories(include ${catkin_INCLUDE_DIRS})

  add_library(${PROJECT_NAME}
    src/my_robot_hw_1.cpp
    src/my_robot_hw_2.cpp
    src/my_robot_hw_3.cpp
    src/my_robot_hw_4.cpp
  )
  target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES})

  add_executable(combined_robot_hw_dummy_app EXCLUDE_FROM_ALL src/dummy_app.cpp)
  target_link_libraries(combined_robot_hw_dummy_app ${PROJECT_NAME} ${catkin_LIBRARIES})
  target_link_libraries(combined_robot_hw_test ${PROJECT_NAME} ${catkin_LIBRARIES})
  add_dependencies(combined_robot_hw_cm_test combined_robot_hw_dummy_app ${catkin_EXPORTED_TARGETS})
  target_link_libraries(combined_robot_hw_cm_test ${PROJECT_NAME} ${catkin_LIBRARIES})

  add_rostest_gtest(combined_robot_hw_test
    test/combined_robot_hw_test.test
    test/combined_robot_hw_test.cpp
  )

  add_rostest_gtest(combined_robot_hw_cm_test
    test/cm_test.test
    test/cm_test.cpp
  )
endif()

#############
## Install ##
#############

# NOTE: Libraries and plugins required for tests are installed since CI
# runs tests out of the install space rather than the devel space

if(BUILD_TESTING)

  # Install targets
  install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
  )

  # Install plugins
  install(FILES test_robot_hw_plugin.xml
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
  )
endif()
