cmake_minimum_required(VERSION 3.5)
project(controller_manager_msgs)

# Load catkin and all dependencies required for this package

find_package(rosidl_default_generators REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
set(CMAKE_CXX_STANDARD 1 4)

# Add message files

# Add service files

rosidl_generate_interfaces(
    ${PROJECT_NAME}
        srv/UnloadController.srv
        srv/SwitchController.srv
        srv/LoadController.srv
        srv/ListControllerTypes.srv
        srv/ListControllers.srv
        srv/ReloadControllerLibraries.srv
        msg/ControllerStatistics.msg
        msg/ControllerState.msg
        msg/ControllersStatistics.msg
        msg/HardwareInterfaceResources.msg
    DEPENDENCIES
        std_msgs
)

# Generate added messages and services with any dependencies listed here

# Declare a catkin package

ament_package()
