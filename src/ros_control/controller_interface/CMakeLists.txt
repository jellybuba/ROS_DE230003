cmake_minimum_required(VERSION 3.5)
project(controller_interface)
find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(rclcpp REQUIRED)

# Load catkin and all dependencies required for this package

# Declare a catkin package

###########
## Build ##
###########

# Specify header include paths
include_directories(include)
ament_export_include_directories(include)
ament_export_dependencies(hardware_interface rclcpp)
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
  target_link_libraries(controller_base_test ${catkin_LIBRARIES})
  catkin_add_gmock(controller_base_test test/controller_base_test.cpp)
endif()
