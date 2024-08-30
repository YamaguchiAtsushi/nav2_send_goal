cmake_minimum_required(VERSION 3.8)
project(nav2_send_goal)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
#find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)



find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

include_directories(
  include
)

#add_executable(nav2_send_goal src/nav2_send_goal.cpp)

ament_auto_add_executable(nav2_send_goal src/nav2_send_goal.cpp src/nav2_send_goal_node.cpp)
ament_target_dependencies(nav2_send_goal rclcpp rclcpp_action nav2_msgs nav_msgs tf2 tf2_geometry_msgs)

ament_auto_add_executable(publish_point src/publish_point.cpp)
ament_target_dependencies(publish_point rclcpp tf2_geometry_msgs)

ament_auto_add_executable(move_point src/move_point.cpp)
ament_target_dependencies(move_point rclcpp nav_msgs tf2 tf2_geometry_msgs)

ament_auto_add_executable(publish_area src/publish_area_a.cpp)
ament_target_dependencies(publish_area rclcpp std_msgs)

ament_auto_add_executable(publish_area_a src/publish_area_a.cpp)
ament_target_dependencies(publish_area_a rclcpp std_msgs)

ament_auto_add_executable(publish_area_b src/publish_area_b.cpp)
ament_target_dependencies(publish_area_b rclcpp std_msgs)

ament_auto_add_executable(publish_area_c src/publish_area_c.cpp)
ament_target_dependencies(publish_area_c rclcpp std_msgs)

install(TARGETS
  nav2_send_goal
  DESTINATION lib/${PROJECT_NAME}/
)

install(TARGETS
  publish_point
  DESTINATION lib/${PROJECT_NAME}/
)

install(TARGETS
  move_point
  DESTINATION lib/${PROJECT_NAME}/
)

install(TARGETS
  publish_area
  DESTINATION lib/${PROJECT_NAME})

  install(TARGETS
  publish_area_a
  DESTINATION lib/${PROJECT_NAME})

  install(TARGETS
  publish_area_b
  DESTINATION lib/${PROJECT_NAME})

  install(TARGETS
  publish_area_c
  DESTINATION lib/${PROJECT_NAME})


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

ament_auto_package(
  INSTALL_TO_SHARE
  launch
)

#ament_package()