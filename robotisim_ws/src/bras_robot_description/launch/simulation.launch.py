import os
import launch
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.actions import ExecuteProcess
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch.actions import DeclareLaunchArgument
from launch_ros.parameter_descriptions import ParameterValue

def generate_launch_description():


    package_name_gazebo = 'bras_robot_description'
    urdf_package = 'bras_robot_description'
    urdf_filename = 'bras_robot_description.urdf'

    pkg_share_description = FindPackageShare(urdf_package)
    default_urdf_model_path = PathJoinSubstitution([pkg_share_description, 'urdf', urdf_filename])

    ros_gz_bridge_config_file_path = 'config/ros_gz_bridge.yaml'
    pkg_share_gazebo = FindPackageShare(package=package_name_gazebo).find(package_name_gazebo)

    urdf_model = LaunchConfiguration('urdf_model')
    use_sim_time = LaunchConfiguration('use_sim_time')

    default_ros_gz_bridge_config_file_path = os.path.join(pkg_share_gazebo, ros_gz_bridge_config_file_path)

    declare_urdf_model_path_cmd = DeclareLaunchArgument(
        name='urdf_model',
        default_value=default_urdf_model_path,
        description='Absolute path to robot urdf file')

    declare_use_sim_time_cmd = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='false',
        description='Use simulation (Gazebo) clock if true')

    os.environ["IGN_GAZEBO_RESOURCE_PATH"] = os.path.join(
        "/home/muneeb/Public/robotics_ws/src/bras_robot_description/models")

    world_file = os.path.join(
        get_package_share_directory('bras_robot_description'),
        'worlds',
        'simulation_world_new.sdf'
    )


    urdf_file = os.path.join(
        get_package_share_directory('bras_robot_description'),
        'urdf',
        'bras_robot_description.urdf'
    )

    with open(urdf_file, 'r') as infp:
        robot_description = infp.read()

    ign_gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('ros_ign_gazebo'),
            'launch',
            'ign_gazebo.launch.py'
        )]),
        launch_arguments={
                'ign_args': f' -v 4 {world_file} '
        }.items()
    )


    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=['/camera@sensor_msgs/msg/Image@ignition.msgs.Image'],
    )

    robot_description_content = ParameterValue(urdf_file, value_type=str)

    start_robot_state_publisher_cmd = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description_content}])

    spawn_robot = Node(
        package='ros_gz_sim',
        executable='create',
        name='spawn_robot',
        output='screen',
        arguments=['-string', robot_description, '-name', 'your_robot', '-x', '0.21', '-y', '0', '-z', '1.04', '-R', '-0.02', '-P', '-0.48', '-Y', '-3.13']
    )

    start_gazebo_ros_bridge_cmd = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        parameters=[{
          'config_file': default_ros_gz_bridge_config_file_path,
        }],
        output='screen'
    )


    return launch.LaunchDescription([
        ign_gazebo,
        start_robot_state_publisher_cmd,
        start_gazebo_ros_bridge_cmd,
        declare_use_sim_time_cmd,
        declare_urdf_model_path_cmd,
        spawn_robot,
        bridge
    ])

