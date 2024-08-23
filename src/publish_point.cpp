#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

class PosePublisher : public rclcpp::Node
{
public:
    PosePublisher() : Node("pose_publisher")
    {
        publisher_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("pose", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&PosePublisher::publish_pose, this));
    }

private:
    void publish_pose()
    {
        auto message = geometry_msgs::msg::PoseStamped();
        message.header.stamp = this->get_clock()->now();
        message.header.frame_id = "map";

        // 座標と姿勢を設定
        message.pose.position.x = 3.0;
        message.pose.position.y = -0.5;
        message.pose.position.z = 0.0;

        message.pose.orientation.x = 0.0;
        message.pose.orientation.y = 0.0;
        message.pose.orientation.z = 0.0;
        message.pose.orientation.w = 1.0;

        RCLCPP_INFO(this->get_logger(), "Publishing: '(%f, %f, %f)'", 
                    message.pose.position.x, 
                    message.pose.position.y, 
                    message.pose.position.z);

        publisher_->publish(message);
    }

    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PosePublisher>());
    rclcpp::shutdown();
    return 0;
}