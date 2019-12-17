/*
 * Copyright (c) 2016, The Regents of the University of California (Regents).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *    3. Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Please contact the author(s) of this library if you have any questions.
 * Authors: Erik Nelson            ( eanelson@eecs.berkeley.edu )
 */

#ifndef POINT_CLOUD_ODOMETRY_H
#define POINT_CLOUD_ODOMETRY_H

#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <nav_msgs/Odometry.h>
#include <geometry_utils/Transform3.h>
#include <pcl_ros/point_cloud.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf/transform_broadcaster.h>
#include <eigen_conversions/eigen_msg.h>
#include <pcl/registration/gicp.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl/search/impl/search.hpp>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_utils/GeometryUtilsROS.h>
#include <parameter_utils/ParameterUtils.h>

class PointCloudOdometry {

public:

  typedef pcl::PointCloud<pcl::PointXYZI> PointCloud;

  PointCloudOdometry();
  ~PointCloudOdometry();

  bool Initialize(const ros::NodeHandle& n);

  bool SetLidar(const PointCloud& points);
  bool SetImuQuaternion(const Eigen::Quaterniond& imu_quaternion);
  bool UpdateEstimate();
  
  const geometry_utils::Transform3& GetIncrementalEstimate() const;
  const geometry_utils::Transform3& GetIntegratedEstimate() const;
  geometry_utils::Transform3 incremental_estimate_;
  geometry_utils::Transform3 integrated_estimate_;
  
  bool GetLastPointCloud(PointCloud::Ptr& out) const;
  double icpFitnessScore_;

  // Aligned point cloud returned by ICP
  PointCloud icpAlignedPointsOdometry_;

private:

  bool LoadParameters(const ros::NodeHandle& n);
  bool RegisterCallbacks(const ros::NodeHandle& n);

  // Use ICP between a query and reference point cloud to estimate pose
  bool UpdateICP();
  
  // Publish reference and query point clouds - TODO: Do we need it? 
  void PublishPoints(const PointCloud::Ptr& points, 
                     const ros::Publisher& pub);

  // Publish incremental and integrated pose estimates
  void PublishPose(const geometry_utils::Transform3& pose,
                   const ros::Publisher& pub);

  // The node's name
  std::string name_;
  bool b_verbose_;

  // For initialization
  bool initialized_;

  // Publishers
  ros::Publisher reference_pub_;
  ros::Publisher query_pub_;
  ros::Publisher incremental_estimate_pub_;
  ros::Publisher integrated_estimate_pub_;

  // Most recent point cloud time stamp for publishers
  ros::Time stamp_;

  // Coordinate frames
  std::string fixed_frame_id_;
  std::string odometry_frame_id_;

  // Point cloud containers
  PointCloud::Ptr query_;
  PointCloud::Ptr reference_;

  // Maximum acceptable translation and rotation tolerances 
  // If transform_thresholding_ is set to false, 
  // neither of these thresholds are considered
  bool transform_thresholding_;
  double max_translation_;
  double max_rotation_;

  // ICP
  struct Parameters {
    double icp_tf_epsilon;
    double icp_corr_dist;
    unsigned int icp_iterations;
  } params_;
  pcl::GeneralizedIterativeClosestPoint<pcl::PointXYZI, pcl::PointXYZI> icp_;
  bool SetupICP();

  // IMU Frontend Integration
  PointCloud points_;
  Eigen::Quaterniond imu_quaternion_;
  Eigen::Quaterniond imu_quaternion_previous_;
  Eigen::Quaterniond imu_quaternion_change_;
  Eigen::Matrix3d GetExternalAttitudeYawChange();
  Eigen::Matrix3d GetExternalAttitudeChange();
  bool b_use_imu_integration_;
  bool b_use_imu_yaw_only_;

};

#endif