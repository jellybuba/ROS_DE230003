cmake_minimum_required(VERSION 3.5)

project(robotti_5)
find_package(ament_cmake REQUIRED)

find_package(roslaunch)
ament_package()

foreach(dir config launch meshes urdf)
	install(DIRECTORY ${dir}/
		DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/${dir})
endforeach(dir)
