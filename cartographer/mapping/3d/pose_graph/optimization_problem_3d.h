/*
 * Copyright 2016 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CARTOGRAPHER_MAPPING_3D_POSE_GRAPH_OPTIMIZATION_PROBLEM_3D_H_
#define CARTOGRAPHER_MAPPING_3D_POSE_GRAPH_OPTIMIZATION_PROBLEM_3D_H_

#include <array>
#include <map>
#include <set>
#include <vector>

#include "Eigen/Core"
#include "Eigen/Geometry"
#include "cartographer/common/optional.h"
#include "cartographer/common/port.h"
#include "cartographer/common/time.h"
#include "cartographer/mapping/id.h"
#include "cartographer/mapping/pose_graph/proto/optimization_problem_options.pb.h"
#include "cartographer/mapping/pose_graph_interface.h"
#include "cartographer/sensor/fixed_frame_pose_data.h"
#include "cartographer/sensor/imu_data.h"
#include "cartographer/sensor/map_by_time.h"
#include "cartographer/sensor/odometry_data.h"
#include "cartographer/transform/transform_interpolation_buffer.h"

namespace cartographer {
namespace mapping {
namespace pose_graph {

// Implements the SPA loop closure method.
class OptimizationProblem3D {
 public:
  using Constraint = PoseGraphInterface::Constraint;
  using LandmarkNode = PoseGraphInterface::LandmarkNode;

  struct NodeData {
    common::Time time;
    transform::Rigid3d local_pose;
    transform::Rigid3d global_pose;
  };

  struct SubmapData {
    transform::Rigid3d global_pose;
  };

  enum class FixZ { kYes, kNo };

  OptimizationProblem3D(
      const pose_graph::proto::OptimizationProblemOptions& options, FixZ fix_z);
  ~OptimizationProblem3D();

  OptimizationProblem3D(const OptimizationProblem3D&) = delete;
  OptimizationProblem3D& operator=(const OptimizationProblem3D&) = delete;

  void AddImuData(int trajectory_id, const sensor::ImuData& imu_data);
  void AddOdometryData(int trajectory_id,
                       const sensor::OdometryData& odometry_data);
  void AddFixedFramePoseData(
      int trajectory_id,
      const sensor::FixedFramePoseData& fixed_frame_pose_data);
  void AddTrajectoryNode(int trajectory_id, common::Time time,
                         const transform::Rigid3d& local_pose,
                         const transform::Rigid3d& global_pose);
  void SetTrajectoryData(
      int trajectory_id,
      const PoseGraphInterface::TrajectoryData& trajectory_data);
  void InsertTrajectoryNode(const NodeId& node_id, common::Time time,
                            const transform::Rigid3d& local_pose,
                            const transform::Rigid3d& global_pose);
  void TrimTrajectoryNode(const NodeId& node_id);
  void AddSubmap(int trajectory_id,
                 const transform::Rigid3d& global_submap_pose);
  void InsertSubmap(const SubmapId& submap_id,
                    const transform::Rigid3d& global_submap_pose);
  void TrimSubmap(const SubmapId& submap_id);

  void SetMaxNumIterations(int32 max_num_iterations);

  // Optimizes the global poses.
  void Solve(const std::vector<Constraint>& constraints,
             const std::set<int>& frozen_trajectories,
             const std::map<std::string, LandmarkNode>& landmark_nodes);

  const MapById<NodeId, NodeData>& node_data() const;
  const MapById<SubmapId, SubmapData>& submap_data() const;
  const std::map<std::string, transform::Rigid3d>& landmark_data() const;
  const sensor::MapByTime<sensor::ImuData>& imu_data() const;
  const sensor::MapByTime<sensor::OdometryData>& odometry_data() const;
  const sensor::MapByTime<sensor::FixedFramePoseData>& fixed_frame_pose_data()
      const;
  const std::map<int, PoseGraphInterface::TrajectoryData>& trajectory_data()
      const;

 private:
  // Uses odometry if available, otherwise the local SLAM results.
  transform::Rigid3d ComputeRelativePose(
      int trajectory_id, const NodeData& first_node_data,
      const NodeData& second_node_data) const;

  pose_graph::proto::OptimizationProblemOptions options_;
  FixZ fix_z_;
  MapById<NodeId, NodeData> node_data_;
  MapById<SubmapId, SubmapData> submap_data_;
  std::map<std::string, transform::Rigid3d> landmark_data_;
  sensor::MapByTime<sensor::ImuData> imu_data_;
  sensor::MapByTime<sensor::OdometryData> odometry_data_;
  sensor::MapByTime<sensor::FixedFramePoseData> fixed_frame_pose_data_;
  std::map<int, PoseGraphInterface::TrajectoryData> trajectory_data_;
};

}  // namespace pose_graph
}  // namespace mapping
}  // namespace cartographer

#endif  // CARTOGRAPHER_MAPPING_3D_POSE_GRAPH_OPTIMIZATION_PROBLEM_3D_H_
