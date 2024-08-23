#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

class MoveToGoal : public rclcpp::Node
{
public:
    MoveToGoal() : Node("move_to_goal")
    {
        // サブスクライバーの設定
        pose_subscriber_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "pose", 10, std::bind(&MoveToGoal::pose_callback, this, std::placeholders::_1));

        odom_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "odom", 10, std::bind(&MoveToGoal::odom_callback, this, std::placeholders::_1));

        // パブリッシャーの設定
        velocity_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

        // ロボットの現在位置と向きの初期化
        current_pose_.position.x = 0.0;
        current_pose_.position.y = 0.0;
        current_yaw_ = 0.0;
    }

private:
    void pose_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
    {
        // 目標座標を取得
        double goal_x = msg->pose.position.x;
        double goal_y = msg->pose.position.y;

        // 現在位置との距離と角度を計算
        double delta_x = goal_x - current_pose_.position.x;
        double delta_y = goal_y - current_pose_.position.y;
        double distance = std::sqrt(delta_x * delta_x + delta_y * delta_y);
        double target_angle = std::atan2(delta_y, delta_x);

        // 目標座標に向かうためのTwistメッセージを作成
        auto twist_msg = geometry_msgs::msg::Twist();

        // 距離が十分に小さい場合、停止
        if (distance < 0.2)
        {
            twist_msg.linear.x = 0.0;
            twist_msg.angular.z = 0.0;
        }
        else
        {
            // 前進速度と回転速度を設定
            twist_msg.linear.x = 0.05 * distance;
            twist_msg.angular.z = 0.02 * (target_angle - current_yaw_);
        }

        velocity_publisher_->publish(twist_msg);
    }

    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        // オドメトリ情報から現在の位置と向きを取得
        current_pose_ = msg->pose.pose;
        tf2::Quaternion q(
            current_pose_.orientation.x,
            current_pose_.orientation.y,
            current_pose_.orientation.z,
            current_pose_.orientation.w);
        tf2::Matrix3x3 m(q);
        double roll, pitch;
        m.getRPY(roll, pitch, current_yaw_);
    }

    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_subscriber_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr velocity_publisher_;

    geometry_msgs::msg::Pose current_pose_;
    double current_yaw_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MoveToGoal>());
    rclcpp::shutdown();
    return 0;
}