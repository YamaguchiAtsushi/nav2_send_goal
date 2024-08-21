#include "nav2_send_goal/nav2_send_goal.hpp"  // include local header

int main(int argc, char** argv){

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Nav2Client>());
  rclcpp::shutdown();

  return 0;
}

