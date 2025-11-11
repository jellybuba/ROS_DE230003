import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
import xacro

def generate_launch_description():

    # Robot name in xacro
    xacroName = 'base_demo'
    pkgName = 'sensors_demo'

    # Model file paths
    relFilePath = 'urdf/base.xacro'
    absModelFile = os.path.join(get_package_share_directory(pkgName),relFilePath)

    # Parse xacro
    robotDesc = xacro.process_file(absModelFile).toxml() # type: ignore

    # Get launch file from ros_gz_sim
    gz_rosPkgLaunch = PythonLaunchDescriptionSource(
        os.path.join(get_package_share_directory('ros_gz_sim'),
                     'launch','gz_sim.launch.py'))

    gzLaunch = IncludeLaunchDescription(gz_rosPkgLaunch,
                launch_arguments={'gz_args':['-r -v -v4 empty.sdf'],
                                  'on_exit_shutdown': 'true'}.items())
    
    # Gazebo node to spawn model
    spawnModelNodeGz = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-name', xacroName,
            '-topic', 'robot_description'
        ],
        output='screen',        
    )

    # Robot state publisher
    nodeRobotStatePublisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robotDesc,
                     'use_sim_time': True}],
        output='screen',     
    )

    relBridge = 'parameters/bridge.yaml'
    bridge_params = os.path.join(get_package_share_directory(pkgName),relBridge)

    gzRosBridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            '--ros-args', '-p',
            f'config_file:={bridge_params}'
        ],
        output='screen', 
    )

    # Create empty launch description object
    launchDescObj = LaunchDescription()

    launchDescObj.add_action(gzLaunch)
    launchDescObj.add_action(spawnModelNodeGz)
    launchDescObj.add_action(nodeRobotStatePublisher)
    launchDescObj.add_action(gzRosBridge)

    return launchDescObj
