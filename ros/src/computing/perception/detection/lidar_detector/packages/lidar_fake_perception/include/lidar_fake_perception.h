/*
*/

#ifndef LIDAR_FAKE_PERCEPTION_H
#define LIDAR_FAKE_PERCEPTION_H

#include <iostream>
#include <vector>

#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <tf_conversions/tf_eigen.h>
#include <sensor_msgs/PointCloud2.h>
#include <geometry_msgs/PoseStamped.h>
#include <autoware_msgs/DetectedObject.h>
#include <autoware_msgs/DetectedObjectArray.h>

#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl_ros/point_cloud.h>
#include <pcl_ros/transforms.h>

class LidarFakePerception
{
public:
  LidarFakePerception();
  ~LidarFakePerception();

  void run();

private:
  // typedefs
  typedef pcl::PointXYZI PointT;
  typedef pcl::PointCloud<PointT> PointCloudT;

  // nodehandle
  ros::NodeHandle nh_;
  ros::NodeHandle private_nh_;

  // publisher
  ros::Publisher fake_objects_pub_;
  ros::Publisher fake_points_pub_;

  // subscriber
  ros::Subscriber object_initial_pose_sub_;
  ros::Subscriber objects_sub_;
  ros::Subscriber points_sub_;

  // tf
  tf::TransformListener tf_listener_;

  // rosparam
  bool publish_object_;
  bool publish_points_;
  double publish_rate_;   // [Hz]
  double object_length_;  // [m]
  double object_width_;  // [m]
  double object_height_;  // [m]
  double object_z_offset_;  // [m]
  double object_velocity_;  // [m/s]
  double object_angular_velocity_;  // [rad/s]
  double object_intensity_;  // 0-255 [-]
  double object_lifetime_;  // minus value -> inifinity [s]
  double object_points_space_;  // [m]
  std::string object_label_;
  std::string object_frame_;

  // variables
  int fake_object_id_;
  bool fake_object_pose_initialized_;
  ros::Time fake_object_initial_time_;
  std::string global_frame_;
  tf::Transform fake_object_pose_;  // global
  autoware_msgs::DetectedObject fake_object_;
  autoware_msgs::DetectedObjectArray fake_objects_;
  autoware_msgs::DetectedObjectArray real_objects_;
  PointCloudT fake_points_;
  PointCloudT real_points_;


  void objectInitialPoseCallback(const geometry_msgs::PoseStampedConstPtr& msg);
  void objectsCallback(const autoware_msgs::DetectedObjectArray& msg);
  void pointsCallback(const sensor_msgs::PointCloud2& msg);
  void updateFakes();
  bool updateFakeObject();
  void updateFakePoints();
  void convertObjectToPoints(const autoware_msgs::DetectedObject& obj, PointCloudT& points);
  void publishFakes();
};

#endif