cmake_minimum_required(VERSION 3.5)
project(imu_sensor_controller)
find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

include_directories(include ${Boost_INCLUDE_DIR})

add_library(${PROJECT_NAME}
  src/imu_sensor_controller.cpp
  include/imu_sensor_controller/imu_sensor_controller.h)
ament_target_dependencies(${PROJECT_NAME} "pluginlib" "controller_interface" "sensor_msgs" "realtime_tools" "hardware_interface")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(controller_interface hardware_interface pluginlib realtime_tools rclcpp sensor_msgs)
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

install(FILES imu_sensor_plugin.xml
              imu_sensor_controller.yaml
              imu_sensor_controller.launch
  DESTINATION share/${PROJECT_NAME}
)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
