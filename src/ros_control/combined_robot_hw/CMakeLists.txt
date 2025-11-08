cmake_minimum_required(VERSION 3.5)
project(combined_robot_hw)
find_package(ament_cmake REQUIRED)
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

add_library(${PROJECT_NAME} src/combined_robot_hw.cpp)

ament_target_dependencies(${PROJECT_NAME} "pluginlib" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(hardware_interface pluginlib rclcpp)
ament_package()

#############
## Install ##
#############

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
