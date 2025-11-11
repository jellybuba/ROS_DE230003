from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import PythonLaunchDescriptionSource

def generate_launch_description():
    
    # Note: Gazebo launch files are not available in ROS2 Jazzy yet
    # This is a placeholder that will need to be updated when gazebo_ros is available
    
    ld = LaunchDescription()
    
    # Placeholder for Gazebo - will need to be updated when gazebo_ros is available in Jazzy
    # gazebo_launch = IncludeLaunchDescription(
    #     PythonLaunchDescriptionSource([
    #         PathJoinSubstitution([
    #             FindPackageShare('gazebo_ros'),
    #             'launch',
    #             'gazebo.launch.py'
    #         ])
    #     ]),
    #     launch_arguments={
    #         'world': PathJoinSubstitution([
    #             FindPackageShare('samk_robowar_world'),
    #             'worlds',
    #             'arena_no_pit.world'
    #         ]),
    #         'gui': 'true',
    #         'verbose': 'true'
    #     }.items()
    # )
    # ld.add_action(gazebo_launch)
    
    # Placeholder for robot spawns
    # robotti_5_launch = IncludeLaunchDescription(
    #     PythonLaunchDescriptionSource([
    #         PathJoinSubstitution([
    #             FindPackageShare('robotti_5'),
    #             'launch',
    #             'spawn_robo5.launch.py'
    #         ])
    #     ])
    # )
    # ld.add_action(robotti_5_launch)
    
    # robotti_1_launch = IncludeLaunchDescription(
    #     PythonLaunchDescriptionSource([
    #         PathJoinSubstitution([
    #             FindPackageShare('robotti_1'),
    #             'launch',
    #             'spawn_robo1.launch.py'
    #         ])
    #     ])
    # )
    # ld.add_action(robotti_1_launch)
    
    print("NOTE: This launch file is a placeholder. Gazebo ROS is not yet available in ROS2 Jazzy.")
    print("To use this launch file, you need to:")
    print("1. Wait for gazebo_ros to be available in Jazzy, OR")
    print("2. Switch to ROS2 Humble which has gazebo_ros support, OR") 
    print("3. Install gazebo_ros from source")
    print("4. Convert robotti_1 and robotti_5 packages to ROS2")
    
    return ld
