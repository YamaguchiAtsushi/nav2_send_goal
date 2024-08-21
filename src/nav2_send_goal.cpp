#include "nav2_send_goal/nav2_send_goal.hpp"  // include local header

Nav2Client::Nav2Client() : rclcpp::Node("nav2_send_goal"), id_(0)
{
  std::cout << "1" << std::endl;

  //follow_type_ = FOLLOW_WAYPOINTS_MODE;//自分でつけた
  follow_type_ = THROUGH_POSES_MODE;//自分でつけた

  std::string action_server_name;

  if (follow_type_ == THROUGH_POSES_MODE){
    action_server_name = "navigate_through_poses";
    nav_through_poses_action_client_ =
        rclcpp_action::create_client<nav2_msgs::action::NavigateThroughPoses>(this, action_server_name);
    rclcpp::sleep_for(500ms);
    is_action_server_ready_ = nav_through_poses_action_client_->wait_for_action_server(std::chrono::seconds(5));

  }else if (follow_type_ == FOLLOW_WAYPOINTS_MODE){
    action_server_name = "follow_waypoints";
    follow_waypoints_action_client_ =
        rclcpp_action::create_client<nav2_msgs::action::FollowWaypoints>(this, action_server_name);
    rclcpp::sleep_for(500ms);
    is_action_server_ready_ = follow_waypoints_action_client_->wait_for_action_server(std::chrono::seconds(5));
  }

  send_goal_options_ = rclcpp_action::Client<nav2_msgs::action::NavigateThroughPoses>::SendGoalOptions();
  send_goal_options_.feedback_callback = std::bind(&Nav2Client::NavThroughPosesFeedbackCallback, this, std::placeholders::_1, std::placeholders::_2);
  send_goal_options_.result_callback = std::bind(&Nav2Client::NavThroughPosesResultCallback, this, std::placeholders::_1);
  send_goal_options_.goal_response_callback = std::bind(&Nav2Client::NavThroughPosesGoalResponseCallback, this, std::placeholders::_1);
  csv_file_ = "/home/yamaguchi-a/turtlebot3_ws/src/nav2_send_goal/csv/waypoints.csv";//自分でつけた
  ReadWaypointsFromCSV(csv_file_, waypoints_);
  start_index_ = (size_t)start_index_int_;
  start_index_ = 1; //自分でつけた
  if(start_index_ < 1 || start_index_ > waypoints_.size()){
    RCLCPP_ERROR(get_logger(), "Invalid start_index");
    std::cout << "start_index_:" << start_index_ << "waypoints_.size()" << waypoints_.size() << std::endl;
    return;
  }

  if (is_action_server_ready_){
     timer_ = create_wall_timer(100ms, std::bind(&Nav2Client::SendWaypointsTimerCallback, this));
  }else{
    RCLCPP_ERROR(this->get_logger(),
                  "%s action server is not available."
                  " Is the initial pose set?"
                  " SendWaypoints was not executed.",
                  action_server_name.c_str());
    return;
  }
}

geometry_msgs::msg::Quaternion rpyYawToQuat(double yaw){
  tf2::Quaternion tf_quat;
  geometry_msgs::msg::Quaternion msg_quat;
  tf_quat.setRPY(0.0, 0.0 ,yaw);
  msg_quat = tf2::toMsg(tf_quat);
  return msg_quat;
}

std::vector<std::string> Nav2Client::getCSVLine(std::string& input, char delimiter)
{
  std::cout << "2" << std::endl;
  
  std::istringstream stream(input);
  std::string field;
  std::vector<std::string> result;
  while (getline(stream, field, delimiter))
  {
    result.push_back(field);
  }
  return result;
}

void Nav2Client::ReadWaypointsFromCSV(std::string& csv_file, std::vector<waypoint_info>& waypoints)
{
  std::cout << "3" << std::endl;

  std::ifstream ifs(csv_file);
  std::string line;
  waypoint_info waypoint;

  while (getline(ifs, line))
  {
    id_++;
    std::vector<std::string> strvec = getCSVLine(line, ',');
    waypoint.poses.position.x = std::stod(strvec.at(0));
    waypoint.poses.position.y = std::stod(strvec.at(1));
    waypoint.poses.position.z = 0.0;
    waypoint.poses.orientation = rpyYawToQuat(std::stod(strvec.at(2))/180.0*M_PI);
    waypoint.will_stop = ("1"==strvec.at(5));

    // std::cout << "-------------------------------------" << std::endl;
    // std::cout << "waypoint ID: " << id << std::endl;
    // std::cout << "trans x: " << std::stod(strvec.at(0)) << std::endl;
    // std::cout << "trans y: " << std::stod(strvec.at(1)) << std::endl;
    // std::cout << "rot yaw: " << std::stod(strvec.at(2)) << std::endl;
    // std::cout << "will stop: "<< std::boolalpha <<  ("1"==strvec.at(5)) << std::endl;

    waypoints.push_back(waypoint);
  }
}


void Nav2Client::SendWaypointsTimerCallback(){
  std::cout << "4" << std::endl;
  static size_t sending_index = start_index_ - 1;
  if(sending_index < waypoints_.size()){
    sending_index = SendWaypointsOnce(sending_index);
  }
}


size_t Nav2Client::SendWaypointsOnce(size_t sending_index){
  std::cout << "5" << std::endl;

  size_t i;
  size_t next_index;
  nav2_msgs::action::NavigateThroughPoses::Goal nav_through_poses_goal;
  nav2_msgs::action::FollowWaypoints::Goal follow_waypoints_goal;
  for(i = sending_index; i<waypoints_.size(); i++){
    std::cout << "5-1" << std::endl;

    geometry_msgs::msg::PoseStamped goal_msg;
    goal_msg.header.stamp = this->now();
    goal_msg.header.frame_id = "map";
    goal_msg.pose=waypoints_[i].poses;
    nav_through_poses_goal.poses.push_back(goal_msg);
    follow_waypoints_goal.poses.push_back(goal_msg);
    if(waypoints_[i].will_stop)break;
  }
  next_index = i + 1;
  if (follow_type_ == THROUGH_POSES_MODE){
    is_goal_achieved_ = false;
    is_goal_accepted_ = false;
    is_aborted_ = false;

    std::chrono::milliseconds server_timeout(1000);
    future_goal_handle_ = nav_through_poses_action_client_->async_send_goal(nav_through_poses_goal, send_goal_options_);
    RCLCPP_INFO(this->get_logger(),
              "[nav_through_poses]: Sending a path of %zu waypoints:", nav_through_poses_goal.poses.size());
  }

  if (follow_type_ == FOLLOW_WAYPOINTS_MODE){
    auto send_goal_options = rclcpp_action::Client<nav2_msgs::action::FollowWaypoints>::SendGoalOptions();
    send_goal_options.result_callback = [this](auto) { follow_waypoints_goal_handle_.reset(); };

    std::chrono::milliseconds server_timeout(1000);
    auto future_goal_handle =
        follow_waypoints_action_client_->async_send_goal(follow_waypoints_goal, send_goal_options);
    if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future_goal_handle, server_timeout) !=
        rclcpp::FutureReturnCode::SUCCESS)
    {
      RCLCPP_ERROR(this->get_logger(), "Send goal call failed");
      //return;
    }
    // Get the goal handle and save so that we can check on completion in the timer callback
    follow_waypoints_goal_handle_ = future_goal_handle.get();
    if (!follow_waypoints_goal_handle_)
    {
      RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
    }
    RCLCPP_INFO(this->get_logger(),
                "[follow_waypoints]: Sending a path of %zu waypoints:", follow_waypoints_goal.poses.size());
  }
  return next_index;
}




void Nav2Client::NavThroughPosesResultCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateThroughPoses>::WrappedResult & result){
  std::cout << "6" << std::endl;

  nav_through_poses_goal_handle_.reset();
  switch (result.code)
  {
    case rclcpp_action::ResultCode::SUCCEEDED:
      RCLCPP_INFO(this->get_logger(), "Goal succeeded!");
      is_goal_achieved_ = true;
      break;

    case rclcpp_action::ResultCode::ABORTED:
      RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
      is_aborted_ = true;
      return;
    
    case rclcpp_action::ResultCode::CANCELED:
      RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
      return;
    
    default:
      RCLCPP_ERROR(this->get_logger(), "Unknown result code");
      return;
  }
}
void Nav2Client::NavThroughPosesGoalResponseCallback(std::shared_ptr<rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateThroughPoses>> future){
  std::cout << "7" << std::endl;

  auto handle = future.get();
  if (!handle)
  {
    RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
    timer_->cancel();
    return ;
  }
  is_goal_accepted_ = true; 
}

void Nav2Client::NavThroughPosesFeedbackCallback(const GoalHandleNavigateNavigateThroughPoses::SharedPtr, const std::shared_ptr<const NavigateThroughPoses::Feedback> feedback){
  std::cout << "8" << std::endl;

  number_of_poses_remaining_ = feedback->number_of_poses_remaining;
  //RCLCPP_INFO(get_logger(), "number of poses remaining = %zu", (size_t)feedback->number_of_poses_remaining);
}

