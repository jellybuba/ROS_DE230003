cmake_minimum_required(VERSION 3.5)
project(transmission_interface)

# Load catkin and all dependencies required for this package

# Include a custom cmake file for TinyXML
find_package(TinyXML REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(cmake_modules REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)

# Declare a catkin package

###########
## Build ##
###########

# Specify header include paths
include_directories(include ${TinyXML_INCLUDE_DIRS})

# Transmission parser library
add_library(${PROJECT_NAME}_parser
  include/transmission_interface/transmission_parser.h
  src/transmission_parser.cpp
)
ament_target_dependencies(${PROJECT_NAME}_parser ${TinyXML_LIBRARIES} "pluginlib" "hardware_interface" "rclcpp")

# Transmission loader library
add_library(${PROJECT_NAME}_loader
  include/transmission_interface/transmission_interface_loader.h
  include/transmission_interface/transmission_loader.h
  src/transmission_interface_loader.cpp
  src/transmission_loader.cpp
)
ament_target_dependencies(${PROJECT_NAME}_loader
  ${PROJECT_NAME}_parser
  ${TinyXML_LIBRARIES}
  "pluginlib"
  "hardware_interface"
  "rclcpp"
)

add_library(${PROJECT_NAME}_loader_plugins
  include/transmission_interface/bidirectional_effort_joint_interface_provider.h
  include/transmission_interface/bidirectional_position_joint_interface_provider.h
  include/transmission_interface/bidirectional_velocity_joint_interface_provider.h
  include/transmission_interface/differential_transmission_loader.h
  include/transmission_interface/effort_joint_interface_provider.h
  include/transmission_interface/four_bar_linkage_transmission_loader.h
  include/transmission_interface/joint_state_interface_provider.h
  include/transmission_interface/position_joint_interface_provider.h
  include/transmission_interface/simple_transmission_loader.h
  include/transmission_interface/velocity_joint_interface_provider.h
  src/bidirectional_effort_joint_interface_provider.cpp
  src/bidirectional_position_joint_interface_provider.cpp
  src/bidirectional_velocity_joint_interface_provider.cpp
  src/differential_transmission_loader.cpp
  src/effort_joint_interface_provider.cpp
  src/four_bar_linkage_transmission_loader.cpp
  src/joint_state_interface_provider.cpp
  src/position_joint_interface_provider.cpp
  src/simple_transmission_loader.cpp
  src/velocity_joint_interface_provider.cpp
)
ament_target_dependencies(${PROJECT_NAME}_loader_plugins ${PROJECT_NAME}_loader)
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME}_parser ${PROJECT_NAME}_loader ${PROJECT_NAME}_loader_plugins)
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
install(TARGETS ${PROJECT_NAME}_loader
                ${PROJECT_NAME}_loader_plugins
                ${PROJECT_NAME}_parser
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install plugins
install(FILES ros_control_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(resource_retriever REQUIRED)
  include_directories(${resource_retriever_INCLUDE_DIRS})
  target_link_libraries(transmission_interface_test               ${catkin_LIBRARIES})
  target_link_libraries(transmission_parser_test                  ${PROJECT_NAME}_parser ${resource_retriever_LIBRARIES})
  target_link_libraries(simple_transmission_loader_test           ${PROJECT_NAME}_parser ${resource_retriever_LIBRARIES})
  target_link_libraries(differential_transmission_loader_test     ${PROJECT_NAME}_parser ${resource_retriever_LIBRARIES})
  target_link_libraries(four_bar_linkage_transmission_loader_test ${PROJECT_NAME}_parser ${resource_retriever_LIBRARIES})
  target_link_libraries(transmission_interface_loader_test        transmission_interface_loader
                                                                  ${resource_retriever_LIBRARIES})

  ament_add_gtest(simple_transmission_test                       test/simple_transmission_test.cpp)

  ament_add_gtest(differential_transmission_test                 test/differential_transmission_test.cpp)

  ament_add_gtest(four_bar_linkage_transmission_test             test/four_bar_linkage_transmission_test.cpp)

  ament_add_gtest(transmission_interface_test                    test/transmission_interface_test.cpp)

  ament_add_gtest(transmission_parser_test                       test/transmission_parser_test.cpp)

  ament_add_gtest(simple_transmission_loader_test                test/simple_transmission_loader_test.cpp)

  ament_add_gtest(differential_transmission_loader_test          test/differential_transmission_loader_test.cpp)

  ament_add_gtest(four_bar_linkage_transmission_loader_test      test/four_bar_linkage_transmission_loader_test.cpp)

  ament_add_gtest(transmission_interface_loader_test             test/transmission_interface_loader_test.cpp)
endif()
