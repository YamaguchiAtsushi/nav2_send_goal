#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class TimerNode : public rclcpp::Node
{
public:
    TimerNode()
        : Node("timer_node"), timer_start_(this->get_clock()->now())
    {
        // タイマーを作成し、コールバック関数を登録
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1), // 1秒ごとにコールバックを実行
            std::bind(&TimerNode::timer_callback, this));
    }

private:
    rclcpp::TimerBase::SharedPtr timer_;  // タイマーを保持する変数
    rclcpp::Time timer_start_;  // 開始時刻を保持する変数
    void timer_callback()
    {
        // 現在時刻を取得
        rclcpp::Time timer_now_ = this->get_clock()->now();
        // 経過時間を計算
        rclcpp::Duration duration = timer_now_ - timer_start_;
        RCLCPP_INFO(this->get_logger(), "Timer callback triggered");
        if (duration > 500ms){
            std::cout << "end" << std::endl;
        } 
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TimerNode>());
    rclcpp::shutdown();
    return 0;
}