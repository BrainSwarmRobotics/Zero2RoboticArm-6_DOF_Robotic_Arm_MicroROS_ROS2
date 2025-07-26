cmake_minimum_required(VERSION 3.8)
project(brainswarm_robotic_arm)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)

install(PROGRAMS
  scripts/trajectory_controller  # Your Python script path
  DESTINATION lib/${PROJECT_NAME}
)

install(PROGRAMS
  scripts/gripper_trajectory_controller  # Your Python script path
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(get_pose src/get_pose.cpp)

ament_target_dependencies(get_pose
  rclcpp
  std_msgs
  geometry_msgs
  moveit_ros_planning_interface
)

install(TARGETS
  get_pose
  DESTINATION lib/${PROJECT_NAME}
)


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

install(DIRECTORY
  config
  meshes
  launch
  scripts
  urdf
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
