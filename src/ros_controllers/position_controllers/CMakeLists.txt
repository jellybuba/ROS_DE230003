cmake_minimum_required(VERSION 3.5)
project(position_controllers)
find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(forward_command_controller REQUIRED)
find_package(pluginlib REQUIRED)

# Load catkin and all dependencies required for this package

# Declare catkin project

include_directories(include ${Boost_INCLUDE_DIR})
add_library(${PROJECT_NAME}
  src/joint_position_controller.cpp
  src/joint_group_position_controller.cpp
)
ament_target_dependencies(${PROJECT_NAME} "forward_command_controller" "pluginlib")
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(controller_interface forward_command_controller)
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

install(FILES position_controllers_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)
