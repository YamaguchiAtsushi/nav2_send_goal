#include "nav2_send_goal/nav2_send_goal.hpp"  // include local header

Nav2Client::Nav2Client() : rclcpp::Node("nav2_send_goal"), id_(0)
{
  //std::cout << "1" << std::endl;

  //follow_type_ = FOLLOW_WAYPOINTS_MODE;//自分でつけた
  follow_type_ = THROUGH_POSES_MODE;//自分でつけた

  rclcpp::QoS latched_qos{ 1 };
  latched_qos.transient_local();
  twist_pub_ = create_publisher<geometry_msgs::msg::Twist>("twist", latched_qos);
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

  waypoints_file_1_ = "/home/yamaguchi-a/turtlebot3_ws/src/nav2_send_goal/csv/waypoints1.csv";//自分でつけた
  waypoints_file_2_ = "/home/yamaguchi-a/turtlebot3_ws/src/nav2_send_goal/csv/waypoints2.csv";//自分でつけた
  waypoints_file_3_ = "/home/yamaguchi-a/turtlebot3_ws/src/nav2_send_goal/csv/waypoints3.csv";//自分でつけた
  waypoints_file_4_ = "/home/yamaguchi-a/turtlebot3_ws/src/nav2_send_goal/csv/waypoints4.csv";//自分でつけた

  csv_file_ = {waypoints_file_1_, waypoints_file_2_, waypoints_file_3_, waypoints_file_4_};//自分で追加

  send_waypoint2_flag = 0;
  send_waypoint3_flag = 0;
  send_waypoint4_flag = 0;



  ReadWaypointsFromCSV(csv_file_[0], waypoints_1_);
  start_index_ = (size_t)start_index_int_;
  start_index_ = 1; //自分でつけた
  if(start_index_ < 1 || start_index_ > waypoints_1_.size()){
    RCLCPP_ERROR(get_logger(), "Invalid start_index");
    std::cout << "start_index_:" << start_index_ << "waypoints_.size()" << waypoints_1_.size() << std::endl;
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
  //std::cout << "2" << std::endl;
  
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
  //std::cout << "3" << std::endl;

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
  //static int state = SEND_WAYPOINTS;
  static int state = SEND_WAYPOINTS1;


  /*
  auto msg = geometry_msgs::msg::Twist();
  for (int i = 0; i< 3; i++){
    rclcpp::sleep_for(500ms);
    msg.linear.x = 0.0;
    msg.angular.z = 0.25;
    twist_pub_->publish(msg);
    std::cout << "rotation" << std::endl;
  }
  */



  std::cout << "sending_index" << sending_index << std::endl;

  switch (state)
  {
  case SEND_WAYPOINTS1:
    if(sending_index < waypoints_1_.size()){
      sending_index =  SendWaypointsOnce(sending_index, waypoints_1_);
    }
    if(is_goal_achieved_){//trueになったら
      state = SEND_WAYPOINTS2;
      is_goal_achieved_ = false;
      is_goal_accepted_ = false;
      is_aborted_ = false;
    }
    break;

  case SEND_WAYPOINTS2:
    std::cout << "SEND_WAYPOINYS2" << std::endl;
    if (send_waypoint2_flag == 0){
      ReadWaypointsFromCSV(csv_file_[1], waypoints_2_);
    }
    send_waypoint2_flag = 1;
    //sending_index = 0; //いらないかも
    if(sending_index < waypoints_2_.size()){
      sending_index =  SendWaypointsOnce(sending_index, waypoints_2_);
//    if(is_goal_achieved_ == True){
 //     state = SEMD_WAYPOINTS2;
  //  }
    }
    if (is_goal_achieved_) {  // waypoint2に到達したら
      state = SEND_WAYPOINTS3;
      is_goal_achieved_ = false;
      is_goal_accepted_ = false;
      is_aborted_ = false;
    }
    break;

  case SEND_WAYPOINTS3:
    std::cout << "SEND_WAYPOINYS3" << std::endl;
    if (send_waypoint3_flag == 0){
      ReadWaypointsFromCSV(csv_file_[2], waypoints_3_);
    }
    send_waypoint3_flag = 1;
    //sending_index = 0; //いらないかも
    if(sending_index < waypoints_3_.size()){
      sending_index =  SendWaypointsOnce(sending_index, waypoints_3_);
//    if(is_goal_achieved_ == True){
 //     state = SEMD_WAYPOINTS2;
  //  }
    }
    if (is_goal_achieved_) {  // waypoint2に到達したら
      state = SEND_WAYPOINTS4;
      is_goal_achieved_ = false;
      is_goal_accepted_ = false;
      is_aborted_ = false;
    }
    break;

  case SEND_WAYPOINTS4:
    std::cout << "SEND_WAYPOINYS4" << std::endl;
    if (send_waypoint4_flag == 0){
      ReadWaypointsFromCSV(csv_file_[3], waypoints_4_);
    }
    send_waypoint4_flag = 1;
    //sending_index = 0; //いらないかも
    if(sending_index < waypoints_4_.size()){
      sending_index =  SendWaypointsOnce(sending_index, waypoints_4_);
//    if(is_goal_achieved_ == True){
 //     state = SEMD_WAYPOINTS2;
  //  }
    }
    if (is_goal_achieved_) {  // waypoint2に到達したら
      state = FINISH_SENDING;
      is_goal_achieved_ = false;
      is_goal_accepted_ = false;
      is_aborted_ = false;
    }
    break;
  case FINISH_SENDING:
    RCLCPP_INFO(this->get_logger(), "Waypoint sending is Finisihed.");
    timer_->cancel();
  break;

  default:
    RCLCPP_INFO(this->get_logger(), "UNKNOWN ERROR");
    timer_->cancel();
    break;
  }
}

size_t Nav2Client::SendWaypointsOnce(size_t sending_index, std::vector<waypoint_info>& waypoints){
  //std::cout << "5" << std::endl;
    std::cout << "sending_index" << sending_index << std::endl;


  size_t i;
  size_t next_index;
  nav2_msgs::action::NavigateThroughPoses::Goal nav_through_poses_goal;
  nav2_msgs::action::FollowWaypoints::Goal follow_waypoints_goal;
  for(i = sending_index; i<waypoints.size(); i++){
    //std::cout << "5-1" << std::endl;

    geometry_msgs::msg::PoseStamped goal_msg;
    goal_msg.header.stamp = this->now();
    goal_msg.header.frame_id = "map";
    goal_msg.pose=waypoints[i].poses;
    nav_through_poses_goal.poses.push_back(goal_msg);
    follow_waypoints_goal.poses.push_back(goal_msg);
    if(waypoints[i].will_stop)break;
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
  //std::cout << "6" << std::endl;

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
  //std::cout << "7" << std::endl;

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
  //std::cout << "8" << std::endl;

  number_of_poses_remaining_ = feedback->number_of_poses_remaining;
  //RCLCPP_INFO(get_logger(), "number of poses remaining = %zu", (size_t)feedback->number_of_poses_remaining);
}

