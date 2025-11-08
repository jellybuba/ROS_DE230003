cmake_minimum_required(VERSION 3.5)

project(samk_robowar_world)
find_package(ament_cmake REQUIRED)

find_package(roslaunch)
ament_package()

foreach(dir config launch meshes urdf models world)
	install(DIRECTORY ${dir}/
		DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/${dir})
endforeach(dir)
