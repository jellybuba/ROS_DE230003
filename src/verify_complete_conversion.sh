#!/bin/bash

verify_complete() {
    local package_name=$1
    local package_path=$2
    
    echo "=== Complete verification for $package_name ==="
    cd "$package_path"
    
    echo "1. Package format:"
    grep 'package format' package.xml
    if grep -q 'format="3"' package.xml; then
        echo "   ✅ PASS: ROS2 format (3)"
    else
        echo "   ❌ FAIL: Not ROS2 format"
    fi
    
    echo "2. Build tool:"
    grep 'buildtool_depend' package.xml | head -1
    if grep -q 'ament_cmake' package.xml; then
        echo "   ✅ PASS: Uses ament_cmake"
    else
        echo "   ❌ FAIL: Not using ament_cmake"
    fi
    
    echo "3. CMakeLists.txt - catkin removed:"
    if grep -q "find_package.*catkin" CMakeLists.txt; then
        echo "   ❌ FAIL: Still contains catkin find_package"
    else
        echo "   ✅ PASS: catkin find_package removed"
    fi
    
    echo "4. CMakeLists.txt - catkin_package removed:"
    if grep -q "catkin_package" CMakeLists.txt; then
        echo "   ❌ FAIL: Still contains catkin_package"
    else
        echo "   ✅ PASS: catkin_package removed"
    fi
    
    echo "----------------------------------------"
}

verify_complete "sensors_demo" "/home/jellybuba/repos/ROS_DE230003/src/sensors_demo"
verify_complete "samk_robowar_world" "/home/jellybuba/repos/ROS_DE230003/src/samk_robowar_world"
