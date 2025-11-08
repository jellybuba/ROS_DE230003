////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012, hiDOF, INC and Willow Garage, Inc
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the names of Willow Garage, Inc., hiDOF Inc, nor the names of its
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
/*
 * Author: Wim Meeussen
 */

#include "controller_manager/controller_manager.h"
#include <algorithm>
#include <chrono>
#include <mutex>
#include <sstream>
#include <thread>
#include <ros/console.h>
#include <controller_manager/controller_loader.h>
#include <controller_manager_msgs/msg/controller_state.hpp>

namespace controller_manager
{


ControllerManager::ControllerManager(hardware_interface::RobotHW *robot_hw, const rclcpp::Node& nh) :
  robot_hw_(robot_hw),
  root_nh_(nh),
  cm_node_(nh, "controller_manager")
{
  // create controller loader
  controller_loaders_.push_back(
    ControllerLoaderInterfaceSharedPtr(new ControllerLoader<controller_interface::ControllerBase>("controller_interface",
                                                                                                  "controller_interface::ControllerBase") ) );

  // Advertise services (this should be the last thing we do in init)
  srv_list_controllers_ = cm_node_.advertiseService("list_controllers", &ControllerManager::listControllersSrv, this);
  srv_list_controller_types_ = cm_node_.advertiseService("list_controller_types", &ControllerManager::listControllerTypesSrv, this);
  srv_load_controller_ = cm_node_.advertiseService("load_controller", &ControllerManager::loadControllerSrv, this);
  srv_unload_controller_ = cm_node_.advertiseService("unload_controller", &ControllerManager::unloadControllerSrv, this);
  srv_switch_controller_ = cm_node_.advertiseService("switch_controller", &ControllerManager::switchControllerSrv, this);
  srv_reload_libraries_ = cm_node_.advertiseService("reload_controller_libraries", &ControllerManager::reloadControllerLibrariesSrv, this);
}


// Must be realtime safe.
void ControllerManager::update(const rclcpp::Time& time, const rclcpp::Duration& period, bool reset_controllers)
{
  used_by_realtime_ = current_controllers_list_;
  
  // Restart all running controllers if motors are re-enabled
  if (reset_controllers){
    for (const auto& controller : controllers_lists_[used_by_realtime_]){
      if (controller.c->isRunning()){
        controller.c->stopRequest(time);
        controller.c->startRequest(time);
      }
    }
  }


  // Update all controllers
  for (const auto& controller : controllers_lists_[used_by_realtime_])
    controller.c->updateRequest(time, period);

  // there are controllers to start/stop
  if (switch_params_.do_switch)
  {
    manageSwitch(time);
  }
}

controller_interface::ControllerBase* ControllerManager::getControllerByName(const std::string& name)
{
  // Lock recursive mutex in this context
  std::lock_guard<std::recursive_mutex> guard(controllers_lock_);

  for (const auto& controller : controllers_lists_[current_controllers_list_])
  {
    if (controller.info.name == name)
      return controller.c.get();
  }
  return nullptr;
}

void ControllerManager::getControllerNames(std::vector<std::string> &names)
{
  std::lock_guard<std::recursive_mutex> guard(controllers_lock_);
  names.clear();
  for (const auto& controller : controllers_lists_[current_controllers_list_])
  {
    names.push_back(controller.info.name);
  }
}

void ControllerManager::manageSwitch(const rclcpp::Time& time)
{
  // switch hardware interfaces (if any)
  if (!switch_params_.started)
  {
    robot_hw_->doSwitch(switch_start_list_, switch_stop_list_);
    switch_params_.started = true;
  }

  stopControllers(time);

  // start controllers once the switch is fully complete
  if (!switch_params_.start_asap)
  {
    startControllers(time);
  }
  // start controllers as soon as their required joints are done switching
  else
  {
    startControllersAsap(time);
  }
}

void ControllerManager::stopControllers(const rclcpp::Time& time)
{
  // stop controllers
  for (const auto& request : stop_request_)
  {
    if (request->isRunning())
    {
      request->stopRequest(time);
    }
  }
}

void ControllerManager::startControllers(const rclcpp::Time& time)
{
  // start controllers
  if (robot_hw_->switchResult() == hardware_interface::RobotHW::SwitchState::DONE)
  {
    for (const auto& request : start_request_)
    {
      request->startRequest(time);
    }

    switch_params_.do_switch = false;
  }
  // abort controllers in case of error or timeout (if set)
  else if ((robot_hw_->switchResult() == hardware_interface::RobotHW::SwitchState::ERROR) ||
           (switch_params_.timeout > 0.0 &&
            (time - switch_params_.init_time).toSec() > switch_params_.timeout))
  {
    for (const auto& request : start_request_)
    {
      request->abortRequest(time);
    }

    switch_params_.do_switch = false;
  }
  // wait controllers
  else
  {
    for (const auto& request : start_request_)
    {
      request->waitRequest(time);
    }
  }
}

void ControllerManager::startControllersAsap(const rclcpp::Time& time)
{
  // start controllers if possible
  for (const auto& request : start_request_)
  {
    if (!request->isRunning())
    {
      // find the info from this controller
      for (const auto& controller : controllers_lists_[current_controllers_list_])
      {
        if (request == controller.c.get())
        {
          // ready to start
          if (robot_hw_->switchResult(controller.info) == hardware_interface::RobotHW::SwitchState::DONE)
          {
            request->startRequest(time);
          }
          // abort on error or timeout (if set)
          else if ((robot_hw_->switchResult(controller.info) == hardware_interface::RobotHW::SwitchState::ERROR) ||
                   (switch_params_.timeout > 0.0 &&
                    (time - switch_params_.init_time).toSec() > switch_params_.timeout))
          {
            request->abortRequest(time);
          }
          // controller is waiting
          else
          {
            request->waitRequest(time);
          }
        }
        continue;
      }
    }
  }

  // all needed controllers started or aborted, switch done
  if (std::all_of(start_request_.begin(), start_request_.end(),
                  [](controller_interface::ControllerBase *request) {
                    return request->isRunning() || request->isAborted();
                  }))
  {
    switch_params_.do_switch = false;
  }
}

bool ControllerManager::loadController(const std::string& name)
{
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Will load controller '%s'", name.c_str());

  // lock controllers
  std::lock_guard<std::recursive_mutex> guard(controllers_lock_);

  // get reference to controller list
  int free_controllers_list = (current_controllers_list_ + 1) % 2;
  while (rclcpp::ok() && free_controllers_list == used_by_realtime_)
  {
    if (!rclcpp::ok())
    {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(200));
  }
  std::vector<ControllerSpec>
    &from = controllers_lists_[current_controllers_list_],
    &to = controllers_lists_[free_controllers_list];
  to.clear();

  // Copy all controllers from the 'from' list to the 'to' list
  for (const auto& controller : from)
    to.push_back(controller);

  // Checks that we're not duplicating controllers
  for (const auto& controller : to)
  {
    if (controller.info.name == name)
    {
      to.clear();
      RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "A controller named '%s' was already loaded inside the controller manager", name.c_str());
      return false;
    }
  }

  rclcpp::Node c_nh;
  // Constructs the controller
  try{
    c_nh = rclcpp::Node(root_nh_, name);
  }
  catch(std::exception &e) {
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Exception thrown while constructing nodehandle for controller with name '%s':\n%s", name.c_str(), e.what());
    return false;
  }
  catch(...){
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Exception thrown while constructing nodehandle for controller with name '%s'", name.c_str());
    return false;
  }
  controller_interface::ControllerBaseSharedPtr c;
  std::string type;
  if (c_nh.getParam("type", type))
  {
    RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Constructing controller '%s' of type '%s'", name.c_str(), type.c_str());
    try
    {
      // Trying loading the controller using all of our controller loaders. Exit once we've found the first valid loaded controller
      auto it = controller_loaders_.begin();
      while (!c && it != controller_loaders_.end())
      {
        for (const auto& cur_type : (*it)->getDeclaredClasses()){
          if (type == cur_type){
            c = (*it)->createInstance(type);
          }
        }
        ++it;
      }
    }
    catch (const std::runtime_error &ex)
    {
      RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not load class '%s': %s", type.c_str(), ex.what());
    }
  }
  else
  {
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not load controller '%s' because the type was not specified. Did you load the controller configuration on the parameter server (namespace: '%s')?", name.c_str(), c_nh.getNamespace().c_str());
    to.clear();
    return false;
  }

  // checks if controller was constructed
  if (!c)
  {
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not load controller '%s' because controller type '%s' does not exist.",  name.c_str(), type.c_str());
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Use 'rosservice call controller_manager/list_controller_types' to get the available types");
    to.clear();
    return false;
  }

  // Initializes the controller
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Initializing controller '%s'", name.c_str());
  bool initialized;
  controller_interface::ControllerBase::ClaimedResources claimed_resources; // Gets populated during initRequest call
  try{
    initialized = c->initRequest(robot_hw_, root_nh_, c_nh, claimed_resources);
  }
  catch(std::exception &e){
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Exception thrown while initializing controller '%s'.\n%s", name.c_str(), e.what());
    initialized = false;
  }
  catch(...){
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Exception thrown while initializing controller '%s'", name.c_str());
    initialized = false;
  }
  if (!initialized)
  {
    to.clear();
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Initializing controller '%s' failed", name.c_str());
    return false;
  }
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Initialized controller '%s' successful", name.c_str());

  // Adds the controller to the new list
  to.resize(to.size() + 1);
  to.back().info.type = type;
  to.back().info.name = name;
  to.back().info.claimed_resources = claimed_resources;
  to.back().c = c;

  // Destroys the old controllers list when the realtime thread is finished with it.
  int former_current_controllers_list_ = current_controllers_list_;
  current_controllers_list_ = free_controllers_list;
  while (rclcpp::ok() && used_by_realtime_ == former_current_controllers_list_)
  {
    if (!rclcpp::ok())
    {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(200));
  }
  from.clear();

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Successfully load controller '%s'", name.c_str());
  return true;
}


bool ControllerManager::unloadController(const std::string &name)
{
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Will unload controller '%s'", name.c_str());

  // lock the controllers
  std::lock_guard<std::recursive_mutex> guard(controllers_lock_);

  // get reference to controller list
  int free_controllers_list = (current_controllers_list_ + 1) % 2;
  while (rclcpp::ok() && free_controllers_list == used_by_realtime_)
  {
    if (!rclcpp::ok())
    {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(200));
  }
  std::vector<ControllerSpec>
    &from = controllers_lists_[current_controllers_list_],
    &to = controllers_lists_[free_controllers_list];
  to.clear();

  // Transfers the running controllers over, skipping the one to be removed and the running ones.
  bool removed = false;
  for (const auto& controller : from)
  {
    if (controller.info.name == name){
      if (controller.c->isRunning()){
        to.clear();
        RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not unload controller with name '%s' because it is still running",
                  name.c_str());
        return false;
      }
      removed = true;
    }
    else
      to.push_back(controller);
  }

  // Fails if we could not remove the controllers
  if (!removed)
  {
    to.clear();
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not unload controller with name '%s' because no controller with this name exists",
              name.c_str());
    return false;
  }

  // Destroys the old controllers list when the realtime thread is finished with it.
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Realtime switches over to new controller list");
  int former_current_controllers_list_ = current_controllers_list_;
  current_controllers_list_ = free_controllers_list;
  while (rclcpp::ok() && used_by_realtime_ == former_current_controllers_list_)
  {
    if (!rclcpp::ok())
    {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(200));
  }
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Destruct controller");
  from.clear();
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Destruct controller finished");

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Successfully unloaded controller '%s'", name.c_str());
  return true;
}


bool ControllerManager::switchController(const std::vector<std::string>& start_controllers,
                                         const std::vector<std::string>& stop_controllers,
                                         int strictness, bool start_asap, double timeout)
{
  switch_params_ = SwitchParams();

  if (!stop_request_.empty() || !start_request_.empty())
    ROS_FATAL("The internal stop and start request lists are not empty at the beginning of the swithController() call. This should not happen.");

  if (strictness == 0){
    RCLCPP_WARN(rclcpp::get_logger("ControllerManager"), "Controller Manager: To switch controllers you need to specify a strictness level of controller_manager_msgs::srv::SwitchController::STRICT (%d) or ::BEST_EFFORT (%d). Defaulting to ::BEST_EFFORT.",
             controller_manager_msgs::srv::SwitchController::Request::STRICT,
             controller_manager_msgs::srv::SwitchController::Request::BEST_EFFORT);
    strictness = controller_manager_msgs::srv::SwitchController::Request::BEST_EFFORT;
  }

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "switching controllers:");
  for (const auto& controller : start_controllers)
    RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), " - starting controller '%s'", controller.c_str());
  for (const auto& controller : stop_controllers)
    RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), " - stopping controller '%s'", controller.c_str());

  // lock controllers
  std::lock_guard<std::recursive_mutex> guard(controllers_lock_);

  controller_interface::ControllerBase* ct;
  // list all controllers to stop
  for (const auto& controller : stop_controllers)
  {
    ct = getControllerByName(controller);
    if (ct == nullptr){
      if (strictness ==  controller_manager_msgs::srv::SwitchController::Request::STRICT){
        RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not stop controller with name '%s' because no controller with this name exists",
                  controller.c_str());
        stop_request_.clear();
        return false;
      }
      else{
        RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Could not stop controller with name '%s' because no controller with this name exists",
                  controller.c_str());
      }
    }
    else{
      RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Found controller '%s' that needs to be stopped in list of controllers",
                controller.c_str());
      stop_request_.push_back(ct);
    }
  }
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Stop request vector has size %i", (int)stop_request_.size());

  // list all controllers to start
  for (const auto& controller : start_controllers)
  {
    ct = getControllerByName(controller);
    if (ct == nullptr){
      if (strictness ==  controller_manager_msgs::srv::SwitchController::Request::STRICT){
        RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not start controller with name '%s' because no controller with this name exists",
                  controller.c_str());
        stop_request_.clear();
        start_request_.clear();
        return false;
      }
      else{
        RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Could not start controller with name '%s' because no controller with this name exists",
                  controller.c_str());
      }
    }
    else{
      RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Found controller '%s' that needs to be started in list of controllers",
                controller.c_str());
      start_request_.push_back(ct);
    }
  }
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Start request vector has size %i", (int)start_request_.size());

  // Do the resource management checking
  std::list<hardware_interface::ControllerInfo> info_list;
  switch_start_list_.clear();
  switch_stop_list_.clear();

  const auto &controllers = controllers_lists_[current_controllers_list_];
  for (const auto& controller : controllers)
  {
    bool in_stop_list  = false;
    for (const auto& request : stop_request_)
    {
      if (request == controller.c.get())
      {
        in_stop_list = true;
        break;
      }
    }

    bool in_start_list = false;
    for (const auto& request : start_request_)
    {
      if (request == controller.c.get())
      {
        in_start_list = true;
        break;
      }
    }

    const bool is_running = controller.c->isRunning();
    const hardware_interface::ControllerInfo &info = controller.info;

    if(!is_running && in_stop_list){ // check for double stop
      if(strictness ==  controller_manager_msgs::srv::SwitchController::Request::STRICT){
        ROS_ERROR_STREAM("Could not stop controller '" << info.name << "' since it is not running");
        stop_request_.clear();
        start_request_.clear();
        return false;
      } else {
        in_stop_list = false;
      }
    }

    if(is_running && !in_stop_list && in_start_list){ // check for doubled start
      if(strictness ==  controller_manager_msgs::srv::SwitchController::Request::STRICT){
        ROS_ERROR_STREAM("Controller '" << info.name << "' is already running");
        stop_request_.clear();
        start_request_.clear();
        return false;
      } else {
        in_start_list = false;
      }
    }

    if(is_running && in_stop_list && !in_start_list){ // running and real stop
      switch_stop_list_.push_back(info);
    }
    else if(!is_running && !in_stop_list && in_start_list){ // start, but no restart
      switch_start_list_.push_back(info);
     }

    bool add_to_list = is_running;
    if (in_stop_list)
      add_to_list = false;
    if (in_start_list)
      add_to_list = true;

    if (add_to_list)
      info_list.push_back(info);
  }

  bool in_conflict = robot_hw_->checkForConflict(info_list);
  if (in_conflict)
  {
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not switch controllers, due to resource conflict");
    stop_request_.clear();
    start_request_.clear();
    return false;
  }

  if (!robot_hw_->prepareSwitch(switch_start_list_, switch_stop_list_))
  {
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Could not switch controllers. The hardware interface combination for the requested controllers is unfeasible.");
    stop_request_.clear();
    start_request_.clear();
    return false;
  }

  // start the atomic controller switching
  switch_params_.strictness = strictness;
  switch_params_.start_asap = start_asap;
  switch_params_.init_time = rclcpp::Time::now();
  switch_params_.timeout = timeout;
  switch_params_.do_switch = true;

  // wait until switch is finished
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Request atomic controller switch from realtime loop");
  while (rclcpp::ok() && switch_params_.do_switch)
  {
    if (!rclcpp::ok())
    {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  start_request_.clear();
  stop_request_.clear();

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "Successfully switched controllers");
  return true;
}


bool ControllerManager::reloadControllerLibrariesSrv(
  controller_manager_msgs::srv::ReloadControllerLibraries::Request &req,
  controller_manager_msgs::srv::ReloadControllerLibraries::Response &resp)
{
  // lock services
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "reload libraries service called");
  std::lock_guard<std::mutex> guard(services_lock_);
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "reload libraries service locked");

  // only reload libraries if no controllers are running
  std::vector<std::string> controllers;
  getControllerNames(controllers);
  if (!controllers.empty() && !req.force_kill){
    RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Controller manager: Cannot reload controller libraries because there are still %i controllers running", (int)controllers.size());
    resp.ok = false;
    return true;
  }

  // kill running controllers if requested
  if (!controllers.empty()){
    RCLCPP_INFO(rclcpp::get_logger("ControllerManager"), "Controller manager: Killing all running controllers");
    std::vector<std::string> empty;
    if (!switchController(empty,controllers, controller_manager_msgs::srv::SwitchController::Request::BEST_EFFORT)){
      RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Controller manager: Cannot reload controller libraries because failed to stop running controllers");
      resp.ok = false;
      return true;
    }
    for (const auto& controller : controllers){
      if (!unloadController(controller)){
        RCLCPP_ERROR(rclcpp::get_logger("ControllerManager"), "Controller manager: Cannot reload controller libraries because failed to unload controller '%s'",
                  controller.c_str());
        resp.ok = false;
        return true;
      }
    }
    getControllerNames(controllers);
  }
  assert(controllers.empty());

  // Force a reload on all the PluginLoaders (internally, this recreates the plugin loaders)
  for (const auto& controller_loader : controller_loaders_)
  {
    controller_loader->reload();
    RCLCPP_INFO(rclcpp::get_logger("ControllerManager"), "Controller manager: reloaded controller libraries for '%s'", controller_loader->getName().c_str());
  }

  resp.ok = true;

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "reload libraries service finished");
  return true;
}


bool ControllerManager::listControllerTypesSrv(
  controller_manager_msgs::srv::ListControllerTypes::Request &req,
  controller_manager_msgs::srv::ListControllerTypes::Response &resp)
{
  // pretend to use the request
  (void) req;

  // lock services
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "list types service called");
  std::lock_guard<std::mutex> guard(services_lock_);
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "list types service locked");

  for (const auto& controller_loader : controller_loaders_)
  {
    std::vector<std::string> cur_types = controller_loader->getDeclaredClasses();
    for (const auto& cur_type : cur_types)
    {
      resp.types.push_back(cur_type);
      resp.base_classes.push_back(controller_loader->getName());
    }
  }

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "list types service finished");
  return true;
}


bool ControllerManager::listControllersSrv(
  controller_manager_msgs::srv::ListControllers::Request &req,
  controller_manager_msgs::srv::ListControllers::Response &resp)
{
  // pretend to use the request
  (void) req;

  // lock services
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "list controller service called");
  std::lock_guard<std::mutex> services_guard(services_lock_);
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "list controller service locked");

  // lock controllers to get all names/types/states
  std::lock_guard<std::recursive_mutex> controller_guard(controllers_lock_);
  std::vector<ControllerSpec> &controllers = controllers_lists_[current_controllers_list_];
  resp.controller.resize(controllers.size());

  for (size_t i = 0; i < controllers.size(); ++i)
  {
    controller_manager_msgs::msg::ControllerState& cs = resp.controller[i];
    cs.name = controllers[i].info.name;
    cs.type = controllers[i].info.type;

    cs.claimed_resources.clear();
    typedef std::vector<hardware_interface::InterfaceResources> ClaimedResVec;
    typedef ClaimedResVec::const_iterator ClaimedResIt;
    const ClaimedResVec& c_resources = controllers[i].info.claimed_resources;
    for (const auto& c_resource : c_resources)
    {
      controller_manager_msgs::msg::HardwareInterfaceResources iface_res;
      iface_res.hardware_interface = c_resource.hardware_interface;
      std::copy(c_resource.resources.begin(), c_resource.resources.end(), std::back_inserter(iface_res.resources));
      cs.claimed_resources.push_back(iface_res);
    }

    if (controllers[i].c->isInitialized())
    {
      cs.state = "initialized";
    }
    else if (controllers[i].c->isRunning())
    {
      cs.state = "running";
    }
    else if (controllers[i].c->isStopped())
    {
      cs.state = "stopped";
    }
    else if (controllers[i].c->isWaiting())
    {
      cs.state = "waiting";
    }
    else if (controllers[i].c->isAborted())
    {
      cs.state = "aborted";
    }
    else
    {
      // should never happen
      cs.state = "unknown";
    }
  }

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "list controller service finished");
  return true;
}


bool ControllerManager::loadControllerSrv(
  controller_manager_msgs::srv::LoadController::Request &req,
  controller_manager_msgs::srv::LoadController::Response &resp)
{
  // lock services
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "loading service called for controller '%s' ",req.name.c_str());
  std::lock_guard<std::mutex> guard(services_lock_);
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "loading service locked");

  resp.ok = loadController(req.name);

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "loading service finished for controller '%s' ",req.name.c_str());
  return true;
}


bool ControllerManager::unloadControllerSrv(
  controller_manager_msgs::srv::UnloadController::Request &req,
  controller_manager_msgs::srv::UnloadController::Response &resp)
{
  // lock services
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "unloading service called for controller '%s' ",req.name.c_str());
  std::lock_guard<std::mutex> guard(services_lock_);
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "unloading service locked");

  resp.ok = unloadController(req.name);

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "unloading service finished for controller '%s' ",req.name.c_str());
  return true;
}


bool ControllerManager::switchControllerSrv(
  controller_manager_msgs::srv::SwitchController::Request &req,
  controller_manager_msgs::srv::SwitchController::Response &resp)
{
  // lock services
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "switching service called");
  std::lock_guard<std::mutex> guard(services_lock_);
  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "switching service locked");

  resp.ok = switchController(req.start_controllers, req.stop_controllers, req.strictness,
                             req.start_asap, req.timeout);

  RCLCPP_DEBUG(rclcpp::get_logger("ControllerManager"), "switching service finished");
  return true;
}

void ControllerManager::registerControllerLoader(ControllerLoaderInterfaceSharedPtr controller_loader)
{
  controller_loaders_.push_back(controller_loader);
}

}
