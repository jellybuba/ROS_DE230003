#!/usr/bin/env python3
import rospy
import csv
import os
from datetime import datetime
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Range

class PositionRecorder:
    def __init__(self):
        rospy.init_node('position_recorder', anonymous=True)
        
        # Data storage
        self.odom_data = None
        self.us_data = {
            'us1': None, 'us2': None, 'us3': None,
            'us4': None, 'us5': None, 'us6': None
        }
        
        # Create CSV file with timestamp
        current_time = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.filename = f"robot_data_{current_time}.csv"
        
        # Initialize CSV file with headers
        with open(self.filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([
                'odom.stamp', 'odom.orientation.x', 'odom.orientation.y', 
                'odom.orientation.z', 'odom.orientation.w', 'us1.range', 
                'us2.range', 'us3.range', 'us4.range', 'us5.range', 
                'us6.range', 'odom.position.x', 'odom.position.y'
            ])
        
        # Subscribers
        rospy.Subscriber('/odom', Odometry, self.odom_callback)
        rospy.Subscriber('/ultrasonic1', Range, self.us1_callback)
        rospy.Subscriber('/ultrasonic2', Range, self.us2_callback)
        rospy.Subscriber('/ultrasonic3', Range, self.us3_callback)
        rospy.Subscriber('/ultrasonic4', Range, self.us4_callback)
        rospy.Subscriber('/ultrasonic5', Range, self.us5_callback)
        rospy.Subscriber('/ultrasonic6', Range, self.us6_callback)
        
        # Timer for recording data
        self.recording = False
        self.start_time = None
        self.duration = 5  # 5 seconds
        
    def odom_callback(self, msg):
        self.odom_data = msg
        
    def us1_callback(self, msg):
        self.us_data['us1'] = msg.range
        
    def us2_callback(self, msg):
        self.us_data['us2'] = msg.range
        
    def us3_callback(self, msg):
        self.us_data['us3'] = msg.range
        
    def us4_callback(self, msg):
        self.us_data['us4'] = msg.range
        
    def us5_callback(self, msg):
        self.us_data['us5'] = msg.range
        
    def us6_callback(self, msg):
        self.us_data['us6'] = msg.range
        
    def start_recording(self):
        self.recording = True
        self.start_time = rospy.get_time()
        rospy.loginfo("Starting 5-second recording...")
        
        rate = rospy.Rate(10)  # 10 Hz
        
        while not rospy.is_shutdown() and self.recording:
            current_time = rospy.get_time()
            
            # Check if 5 seconds have passed
            if current_time - self.start_time >= self.duration:
                self.recording = False
                rospy.loginfo("Recording completed!")
                break
                
            # Write data if we have all necessary information
            if self.odom_data is not None and all(value is not None for value in self.us_data.values()):
                self.write_to_csv()
            
            rate.sleep()
    
    def write_to_csv(self):
        try:
            with open(self.filename, 'a', newline='') as csvfile:
                writer = csv.writer(csvfile)
                
                # Extract data
                stamp = self.odom_data.header.stamp.to_sec()
                orientation = self.odom_data.pose.pose.orientation
                position = self.odom_data.pose.pose.position
                
                writer.writerow([
                    stamp,
                    orientation.x, orientation.y, orientation.z, orientation.w,
                    self.us_data['us1'], self.us_data['us2'], self.us_data['us3'],
                    self.us_data['us4'], self.us_data['us5'], self.us_data['us6'],
                    position.x, position.y
                ])
                
        except Exception as e:
            rospy.logerr(f"Error writing to CSV: {e}")

if __name__ == '__main__':
    try:
        recorder = PositionRecorder()
        rospy.sleep(1)  # Wait for subscribers to connect
        
        # Wait for some initial data
        rospy.loginfo("Waiting for sensor data...")
        timeout = rospy.get_time() + 10  # 10 second timeout
        
        while (recorder.odom_data is None or 
               any(value is None for value in recorder.us_data.values())):
            if rospy.get_time() > timeout:
                rospy.logwarn("Timeout waiting for sensor data. Starting recording anyway.")
                break
            rospy.sleep(0.1)
        
        # Start recording
        recorder.start_recording()
        
    except rospy.ROSInterruptException:
        pass