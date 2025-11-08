cmake_minimum_required(VERSION 3.5)

project(robotti_1)
find_package(ament_cmake REQUIRED)

find_package(roslaunch)
ament_package()

foreach(dir config launch meshes urdf)
	install(DIRECTORY ${dir}/
		DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/${dir})
endforeach(dir)
