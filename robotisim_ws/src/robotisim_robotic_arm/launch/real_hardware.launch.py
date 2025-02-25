from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, Command
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue  # Import this
import os

def generate_launch_description():
    # Path to URDF
    pkg_share = os.path.join(os.getenv('HOME'), 'Desktop', 'robotisim_ws', 'src', 'robotisim_robotic_arm')
    default_model_path = os.path.join(pkg_share, 'urdf', 'real_hardware.urdf.xacro')

    # Declare arguments
    declared_arguments = [
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation clock if true',
        ),
        DeclareLaunchArgument(
            'robot_description_file',
            default_value=default_model_path,
            description='Path to the URDF file',
        ),
    ]

    # Properly set robot description
    robot_description_content = ParameterValue(
        Command(['xacro', ' ', LaunchConfiguration('robot_description_file')]), value_type=str
    )
    robot_description = {'robot_description': robot_description_content}

    # Nodes

    controller_manager_config = PathJoinSubstitution(
        [
            FindPackageShare('robotisim_robotic_arm'),
            'config',
            'real_robotic_arm_controllers.yaml',
        ]
    )
    control_node = Node(
        package='controller_manager',
        executable='ros2_control_node',
        namespace='real_robot',
        parameters=[
            robot_description,
            controller_manager_config
        ],
        output="both",
    )

    trajectory_controller_node = Node(
        package='robotisim_robotic_arm',
        executable='trajectory_controller',
        namespace='real_robot',
        output="both",
    )

    gripper_trajectory_controller_node = Node(
        package='robotisim_robotic_arm',
        executable='gripper_trajectory_controller',
        namespace='real_robot',
        output="both",
    )


    rviz_config_path = os.path.join(pkg_share, 'rviz', 'default.rviz')

    # Spawner nodes



    return LaunchDescription(declared_arguments + [
        gripper_trajectory_controller_node,
        trajectory_controller_node,
        control_node,
    ])
