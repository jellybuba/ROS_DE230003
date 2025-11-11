cmake_minimum_required(VERSION 3.5)
project(sensors_demo)

# Find ament build dependencies
find_package(ament_cmake REQUIRED)

# Install launch files
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

# Install URDF files  
install(DIRECTORY urdf/
  DESTINATION share/${PROJECT_NAME}/urdf
)

ament_package()
