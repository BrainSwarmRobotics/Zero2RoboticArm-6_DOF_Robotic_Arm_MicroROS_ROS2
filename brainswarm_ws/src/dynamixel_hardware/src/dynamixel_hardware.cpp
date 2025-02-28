#include "dynamixel_hardware/dynamixel_hardware.hpp"
#include <pluginlib/class_list_macros.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <sensor_msgs/msg/joint_state.hpp>


namespace dynamixel_hardware
{

hardware_interface::CallbackReturn DynamixelHardware::on_init(const hardware_interface::HardwareInfo &info)
{
    if (hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS)
    {
        return hardware_interface::CallbackReturn::ERROR;
    }

    num_joints_ = info.joints.size();
    position_commands_.resize(num_joints_, 0.0);
    position_states_.resize(num_joints_, 0.0);

    // ROS2 Node initialization
    node_ = rclcpp::Node::make_shared("dynamixel_hardware");
    executor_ = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    executor_->add_node(node_);
    executor_thread_ = std::thread([this]() { executor_->spin(); });

    //Subscriber to Joint States
    joint_states_subscriber_ = node_->create_subscription<sensor_msgs::msg::JointState>(
      "/joint_states", 10,
      std::bind(&DynamixelHardware::joint_states_callback, this, std::placeholders::_1));

    // Subscriber for encoder_angle
    target_angle_publisher_ = node_->create_publisher<std_msgs::msg::Float64MultiArray>(
        "/motor_command", 10);


    RCLCPP_INFO(node_->get_logger(), "Hardware interface initialized with ROS2 Subscriber for encoder_angle.");
    return hardware_interface::CallbackReturn::SUCCESS;
}

// Callback function to update position_states_
void DynamixelHardware::joint_states_callback(const sensor_msgs::msg::JointState::SharedPtr msg)
{
    if (msg->position.size() >= num_joints_)
    {
        for (size_t i = 0; i < num_joints_; i++)
        {
            position_states_[i] = msg->position[i];  // Update joint positions
        }
    }
}


std::vector<hardware_interface::StateInterface> DynamixelHardware::export_state_interfaces()
{
    std::vector<hardware_interface::StateInterface> state_interfaces;
    for (size_t i = 0; i < num_joints_; i++)
    {
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                info_.joints[i].name, "position", &position_states_[i]));
    }
    return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> DynamixelHardware::export_command_interfaces()
{
    std::vector<hardware_interface::CommandInterface> command_interfaces;
    for (size_t i = 0; i < num_joints_; i++)
    {
        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(
                info_.joints[i].name, "position", &position_commands_[i]));
    }
    return command_interfaces;
}

hardware_interface::return_type DynamixelHardware::read(const rclcpp::Time &, const rclcpp::Duration &)
{
    for (size_t i = 0; i < num_joints_; i++)
    {
        position_states_[i] = position_states_[i];
        RCLCPP_DEBUG(node_->get_logger(), "Joint %zu state: %f", i, position_states_[i]);
    }
    return hardware_interface::return_type::OK;
}

hardware_interface::return_type DynamixelHardware::write(const rclcpp::Time &, const rclcpp::Duration &)
{
    // Create a single message to contain all joint commands
    auto msg = std_msgs::msg::Float64MultiArray();
    msg.data = position_commands_;  // Use the position_commands_ vector directly

    // Publish the complete command array once
    target_angle_publisher_->publish(msg);

    // Optionally, log the commands for debugging purposes
    for (size_t i = 0; i < num_joints_; i++)
    {
        RCLCPP_DEBUG(node_->get_logger(), "Joint %zu command: %f", i, position_commands_[i]);
    }

    return hardware_interface::return_type::OK;
}

void DynamixelHardware::send_command_to_motor(size_t joint_index, double position)
{
    auto msg = std_msgs::msg::Float64MultiArray();
    msg.data.resize(num_joints_);  // Resize to match the number of joints
    msg.data[joint_index] = position;  // Assign the command to the correct index
    target_angle_publisher_->publish(msg);
}



} // namespace dynamixel_hardware

PLUGINLIB_EXPORT_CLASS(dynamixel_hardware::DynamixelHardware, hardware_interface::SystemInterface)

