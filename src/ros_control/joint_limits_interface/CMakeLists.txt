cmake_minimum_required(VERSION 3.5)
project(joint_limits_interface)
find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(rclcpp REQUIRED)
find_package(urdf REQUIRED)

# Load catkin and all dependencies required for this package

# Declare a catkin package

###########
## Build ##
###########

# Specify header include paths
include_directories(include)
ament_export_include_directories(include)
ament_export_dependencies(hardware_interface rclcpp urdf)
ament_package()

#############
## Install ##
#############

# Install headers
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(rostest REQUIRED)
  target_link_libraries(joint_limits_interface_test ${catkin_LIBRARIES})
  target_link_libraries(joint_limits_urdf_test ${catkin_LIBRARIES})
  target_link_libraries(joint_limits_rosparam_test ${catkin_LIBRARIES})

  add_rostest_gtest(joint_limits_rosparam_test
    test/joint_limits_rosparam.test
    test/joint_limits_rosparam_test.cpp
  )

  ament_add_gtest(joint_limits_interface_test test/joint_limits_interface_test.cpp)

  ament_add_gtest(joint_limits_urdf_test test/joint_limits_urdf_test.cpp)
endif()
