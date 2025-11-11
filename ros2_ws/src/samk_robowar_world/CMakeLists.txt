cmake_minimum_required(VERSION 3.5)
project(samk_robowar_world)

# Find dependencies
find_package(ament_cmake REQUIRED)

# Install launch files
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}
)

# Install config files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config")
  install(DIRECTORY config/
    DESTINATION share/${PROJECT_NAME}
  )
endif()

# Install URDF files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/urdf")
  install(DIRECTORY urdf/
    DESTINATION share/${PROJECT_NAME}
  )
endif()

# Install world files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/worlds")
  install(DIRECTORY worlds/
    DESTINATION share/${PROJECT_NAME}
  )
endif()

ament_package()
