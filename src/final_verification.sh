#!/bin/bash

final_check() {
    local package_name=$1
    local package_path=$2
    
    echo "=== FINAL CHECK: $package_name ==="
    cd "$package_path"
    
    all_pass=true
    
    # Check package.xml
    if grep -q 'format="3"' package.xml; then
        echo "✅ Package format: ROS2 (format=3)"
    else
        echo "❌ Package format: NOT ROS2"
        all_pass=false
    fi
    
    if grep -q 'ament_cmake' package.xml; then
        echo "✅ Build tool: ament_cmake"
    else
        echo "❌ Build tool: NOT ament_cmake"
        all_pass=false
    fi
    
    # Check CMakeLists.txt
    if grep -q "catkin" CMakeLists.txt; then
        echo "❌ CMakeLists: Still contains catkin"
        all_pass=false
    else
        echo "✅ CMakeLists: No catkin commands"
    fi
    
    if grep -q "ament" CMakeLists.txt; then
        echo "✅ CMakeLists: Contains ament commands"
    else
        echo "⚠️  CMakeLists: No ament commands (might need completion)"
    fi
    
    if [ "$all_pass" = true ]; then
        echo "🎉 $package_name: FULLY CONVERTED TO ROS2"
    else
        echo "❌ $package_name: CONVERSION INCOMPLETE"
    fi
    echo "----------------------------------------"
}

final_check "sensors_demo" "/home/jellybuba/repos/ROS_DE230003/src/sensors_demo"
final_check "samk_robowar_world" "/home/jellybuba/repos/ROS_DE230003/src/samk_robowar_world"
