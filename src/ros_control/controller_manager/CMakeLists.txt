cmake_minimum_required(VERSION 3.5)
project(controller_manager)
find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(controller_manager_msgs REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)

# Load catkin and all dependencies required for this package

# Declare a catkin package

###########
## Build ##
###########

# Specify header include paths
include_directories(include)

add_library(${PROJECT_NAME}
  include/controller_manager/controller_loader.h
  include/controller_manager/controller_loader_interface.h
  include/controller_manager/controller_manager.h
  src/controller_manager.cpp
)

ament_target_dependencies(${PROJECT_NAME} "rclpy" "rclcpp" "std_msgs" "rosparam" "pluginlib" "controller_interface" "controller_manager_msgs" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(controller_interface controller_manager_msgs hardware_interface pluginlib rclcpp)
ament_package()

# Install headers
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

# Install targets
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

#############
## Install ##
#############

# Install python scripts
catkin_install_python(PROGRAMS scripts/controller_group
                               scripts/controller_manager
                               scripts/spawner
                               scripts/unspawner
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(rostest REQUIRED)
  add_dependencies(${PROJECT_NAME}_hwi_switch_test ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_hwi_switch_test ${PROJECT_NAME} ${catkin_LIBRARIES})
  add_dependencies(${PROJECT_NAME}_hwi_update_test ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_hwi_update_test ${PROJECT_NAME} ${catkin_LIBRARIES})

  add_rostest_gtest(${PROJECT_NAME}_hwi_switch_test
    test/hwi_switch_test.test
    test/hwi_switch_test.cpp
  )

  add_rostest_gmock(${PROJECT_NAME}_hwi_update_test
    test/hwi_update_test.test
    test/hwi_update_test.cpp
  )
endif()
