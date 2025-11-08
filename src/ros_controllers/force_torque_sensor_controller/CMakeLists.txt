cmake_minimum_required(VERSION 3.5)
project(force_torque_sensor_controller)
find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(rclcpp REQUIRED)

# Load catkin and all dependencies required for this package

# Declare catkin package

include_directories(include ${Boost_INCLUDE_DIR})

add_library(${PROJECT_NAME}
  src/force_torque_sensor_controller.cpp
  include/force_torque_sensor_controller/force_torque_sensor_controller.h
)
ament_target_dependencies(${PROJECT_NAME} "geometry_msgs" "pluginlib" "controller_interface" "realtime_tools" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(controller_interface geometry_msgs hardware_interface pluginlib realtime_tools rclcpp)
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

install(FILES force_torque_sensor_plugin.xml
              force_torque_sensor_controller.yaml
              force_torque_sensor_controller.launch
  DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
