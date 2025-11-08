///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013, PAL Robotics S.L.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of PAL Robotics S.L. nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////////////

/// \author Adolfo Rodriguez Tsouroukdissian

#include <algorithm>
#include <cmath>
#include <memory>
#include <mutex>

#include <gtest/gtest.h>

#include "rclcpp/rclcpp.hpp"
#include <actionlib/client/simple_action_client.h>

#include <std_msgs/msg/float64.hpp>
#include <control_msgs/msg/follow_joint_trajectory_action.hpp>
#include <control_msgs/msg/joint_trajectory_controller_state.hpp>
#include <control_msgs/srv/query_trajectory_state.hpp>

// Floating-point value comparison threshold
const double EPS = 0.01;

using actionlib::SimpleClientGoalState;

class JointTrajectoryControllerTest : public ::testing::Test
{
public:
  JointTrajectoryControllerTest()
    : nh("rrbot_wrapping_controller"),
      short_timeout(1.0),
      long_timeout(10.0),
      controller_state()
  {
    n_joints = (2);
    joint_names.resize(n_joints);
    joint_names[0] = "joint1";
    joint_names[1] = "joint2";

    max_pos_err[0] = 0.0;
    max_pos_err[1] = 0.0;

    trajectory_msgs::msg::JointTrajectoryPoint point;
    point.positions.resize(n_joints, 0.0);
    point.velocities.resize(n_joints, 0.0);
    point.accelerations.resize(n_joints, 0.0);

    // Home position just inside upper and lower wrapping angles
    point.positions[0] = M_PI - 0.1;
    point.positions[1] = -M_PI + 0.1;

    // Go home trajectory
    traj_home.joint_names = joint_names;
    traj_home.points.resize(1, point);
    traj_home.points[0].time_from_start = rclcpp::Duration(1.0);

    // Trajectory to command joints outside of wrap range
    points.resize(3, point);
    points[0].positions[0] =  M_PI + 0.01;  // Just outside wrap
    points[0].positions[1] =  -M_PI - 0.01;    // Just outside wrap
    points[0].time_from_start = rclcpp::Duration(1.0);

    points[1].positions[0] =  M_PI + 0.06;
    points[1].positions[1] = -M_PI - 0.06;
    points[1].time_from_start = rclcpp::Duration(2.0);

    points[2].positions[0] = M_PI + 0.1;
    points[2].positions[1] =  -M_PI - 0.01;
    points[2].time_from_start = rclcpp::Duration(4.0);

    traj.joint_names = joint_names;
    traj.points = points;

    // Action goals
    traj_home_goal.trajectory = traj_home;
    traj_goal.trajectory      = traj;

    // Smoothing publisher (determines how well the robot follows a trajectory)
    smoothing_pub = rclcpp::Node().advertise<std_msgs::msg::Float64>("smoothing", 1);

    // Trajectory publisher
    traj_pub = nh.advertise<trajectory_msgs::msg::JointTrajectory>("command", 1);

    // State subscriber
    state_sub = nh.subscribe<control_msgs::msg::JointTrajectoryControllerState>("state",
                                                                           1,
                                                                           &JointTrajectoryControllerTest::stateCB,
                                                                           this);

    // Query state service client
    query_state_service = nh.serviceClient<control_msgs::srv::QueryTrajectoryState>("query_state");

    // Action client
    const std::string action_server_name = nh.getNamespace() + "/follow_joint_trajectory";
    action_client.reset(new ActionClient(action_server_name));
    action_client2.reset(new ActionClient(action_server_name));
  }

  ~JointTrajectoryControllerTest()
  {
    state_sub.shutdown(); // This is important, to make sure that the callback is not woken up later in the destructor
  }

protected:
  typedef actionlib::SimpleActionClient<control_msgs::msg::FollowJointTrajectoryAction> ActionClient;
  typedef std::shared_ptr<ActionClient> ActionClientPtr;
  typedef control_msgs::msg::FollowJointTrajectoryGoal ActionGoal;
  typedef control_msgs::msg::JointTrajectoryControllerState::ConstSharedPtr StateConstPtr;

  std::mutex mutex;
  rclcpp::Node nh;

  unsigned int n_joints;
  std::vector<std::string> joint_names;
  std::vector<trajectory_msgs::msg::JointTrajectoryPoint> points;

  trajectory_msgs::msg::JointTrajectory traj_home;
  trajectory_msgs::msg::JointTrajectory traj;
  ActionGoal                       traj_home_goal;
  ActionGoal                       traj_goal;

  rclcpp::Duration short_timeout;
  rclcpp::Duration long_timeout;

  ros::Publisher     smoothing_pub;
  ros::Publisher     traj_pub;
  ros::Subscriber    state_sub;
  ros::ServiceClient query_state_service;
  ActionClientPtr    action_client;
  ActionClientPtr    action_client2;


  StateConstPtr controller_state;
  double max_pos_err[2];

  void stateCB(const StateConstPtr& state)
  {
    std::lock_guard<std::mutex> lock(mutex);
    controller_state = state;

    // Keep track of maximum position error commands
    if (state->joint_names.size() == 2 && state->desired.velocities.size() == 2) {
        for (int i = 0; i < 2; ++i) {
            if (fabs(max_pos_err[i]) < fabs(state->error.positions[i])) {
                max_pos_err[i] = state->error.positions[i];
            }
        }
    }
  }

  StateConstPtr getState()
  {
    std::lock_guard<std::mutex> lock(mutex);
    return controller_state;
  }

  double getMaxPosErr(const int& idx)
  {
      return max_pos_err[idx];
  }

  bool initState(const rclcpp::Duration& timeout = rclcpp::Duration(5.0))
  {
    bool init_ok = false;
    rclcpp::Time start_time = rclcpp::Time::now();
    while (!init_ok && (rclcpp::Time::now() - start_time) < timeout)
    {
      {
        std::lock_guard<std::mutex> lock(mutex);
        init_ok = controller_state && !controller_state->joint_names.empty();
      }
      rclcpp::Duration(0.1).sleep();
    }
    return init_ok;
  }

  static bool waitForState(const ActionClientPtr& action_client,
                           const actionlib::SimpleClientGoalState& state,
                           const rclcpp::Duration& timeout)
  {
    using rclcpp::Time;
    using rclcpp::Duration;

    Time start_time = Time::now();
    while (action_client->getState() != state && rclcpp::ok())
    {
      if (timeout >= Duration(0.0) && (Time::now() - start_time) > timeout) { return false; } // Timed-out
      rclcpp::Duration(0.01).sleep();
    }
    return true;
  }
};

TEST_F(JointTrajectoryControllerTest, jointWrapping)
{
  ASSERT_TRUE(initState());
  ASSERT_TRUE(action_client->waitForServer(long_timeout));

  // Go to home configuration, we need known initial conditions
  traj_home_goal.trajectory.header.stamp = rclcpp::Time(0); // Start immediately
  action_client->sendGoal(traj_home_goal);
  ASSERT_TRUE(waitForState(action_client, SimpleClientGoalState::SUCCEEDED, long_timeout));

  // Make robot respond with a delay
  {
    std_msgs::msg::Float64 smoothing;
    smoothing.data = 0.75;
    smoothing_pub.publish(smoothing);
    rclcpp::Duration(0.5).sleep();
  }

  // Disable path constraints
  traj_goal.path_tolerance.resize(2);
  traj_goal.path_tolerance[0].name     = "joint1";
  traj_goal.path_tolerance[0].position = -1.0;
  traj_goal.path_tolerance[0].velocity = -1.0;
  traj_goal.path_tolerance[1].name     = "joint2";
  traj_goal.path_tolerance[1].position = -1.0;
  traj_goal.path_tolerance[1].velocity = -1.0;

  // Send trajectory (command joints outside of wrap range)
  traj_goal.trajectory.header.stamp = rclcpp::Time(0); // Start immediately
  action_client->sendGoal(traj_goal);
  EXPECT_TRUE(waitForState(action_client, SimpleClientGoalState::ACTIVE, short_timeout));

  // Wait until done
  EXPECT_TRUE(waitForState(action_client, SimpleClientGoalState::SUCCEEDED, long_timeout));

  // Make sure max position error is small (which would be violated
  // when the position differences are not wrapped properly)
  EXPECT_TRUE(fabs(getMaxPosErr(0)) < 0.7);
  EXPECT_TRUE(fabs(getMaxPosErr(1)) < 0.7);

  // Restore perfect control
  {
    std_msgs::msg::Float64 smoothing;
    smoothing.data = 0.0;
    smoothing_pub.publish(smoothing);
    rclcpp::Duration(0.5).sleep();
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("joint_trajectory_controller_wrapping_test");

  ros::AsyncSpinner spinner(1);
  spinner.start();
  int ret = RUN_ALL_TESTS();
  spinner.stop();
  ros::shutdown();
  return ret;
}
