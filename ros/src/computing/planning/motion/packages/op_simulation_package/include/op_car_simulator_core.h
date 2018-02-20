/*
// *  Copyright (c) 2017, Nagoya University
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name of Autoware nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OP_CAR_SIMULATOR
#define OP_CAR_SIMULATOR

// ROS includes
#include <ros/ros.h>

#include <geometry_msgs/Vector3Stamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/PoseStamped.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <tf/tf.h>
#include <std_msgs/Int8.h>


#include <visualization_msgs/MarkerArray.h>
#include <visualization_msgs/InteractiveMarkerPose.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Float32.h>
#include "autoware_msgs/Signals.h"
#include "autoware_msgs/DetectedObjectArray.h"
#include "autoware_msgs/CloudClusterArray.h"

#include "TrajectoryFollower.h"
#include "LocalPlannerH.h"
#include "PlannerH.h"
#include "MappingHelpers.h"

#include "SimpleTracker.h"
#include "SimuDecisionMaker.h"

namespace CarSimulatorNS
{

enum MAP_SOURCE_TYPE{MAP_AUTOWARE, MAP_FOLDER, MAP_KML_FILE};

class SimuCommandParams
{
public:
	int id;
	std::string 	KmlMapPath;
	std::string 	strID;
	std::string 	meshPath;
	std::string 	logPath;
	MAP_SOURCE_TYPE	mapSource;
	bool			bRvizPositions;
	bool 			bLooper;
	PlannerHNS::WayPoint startPose;
	PlannerHNS::WayPoint goalPose;
	std_msgs::ColorRGBA modelColor;
	bool			bEnableLogs;

	SimuCommandParams()
	{
		id = 1;
		bEnableLogs = false;
		bLooper = false;
		bRvizPositions = true;
		mapSource = MAP_FOLDER;
		modelColor.a = 1;
		modelColor.b = 1;
		modelColor.r = 1;
		modelColor.g = 1;
	}
};

class OpenPlannerCarSimulator
{
protected:
	timespec m_PlanningTimer;
	geometry_msgs::Pose m_OriginPos;


	std::string m_BaseLinkFrameID;
	std::string m_VelodyneFrameID;

	bool m_bStepByStep;
	bool m_bSimulatedVelodyne;
	bool m_bGoNextStep;
	bool 						m_bMap;
	PlannerHNS::RoadNetwork		m_Map;
	PlannerHNS::PlannerH		m_GlobalPlanner;
	PlannerHNS::SimuDecisionMaker* 	m_LocalPlanner;
	SimulationNS::TrajectoryFollower m_PredControl;
	std::vector<PlannerHNS::DetectedObject> m_PredictedObjects;
	std::vector<std::vector<PlannerHNS::WayPoint> > m_GlobalPaths;
	bool bPredictedObjects;

	bool bInitPos;
	bool bGoalPos;

	bool bNewLightSignal;
	std::vector<PlannerHNS::TrafficLight> m_CurrTrafficLight;
	std::vector<PlannerHNS::TrafficLight> m_PrevTrafficLight;


	SimuCommandParams m_SimParams;
	PlannerHNS::CAR_BASIC_INFO m_CarInfo;
	PlannerHNS::ControllerParams m_ControlParams;
	PlannerHNS::PlanningParams m_PlanningParams;
	PlannerHNS::BehaviorState m_CurrBehavior;

	ros::NodeHandle nh;

	tf::TransformListener m_Listener;

	ros::Publisher pub_SafetyBorderRviz;
	ros::Publisher pub_SimuBoxPose;
	ros::Publisher pub_CurrPoseRviz;
	ros::Publisher pub_LocalTrajectoriesRviz;
	ros::Publisher pub_BehaviorStateRviz;
	ros::Publisher pub_PointerBehaviorStateRviz;
	ros::Publisher pub_InternalInfoRviz;
	ros::Publisher pub_SimulatedVelodyne;

	// define subscribers.
	ros::Subscriber sub_initialpose;
	ros::Subscriber sub_goalpose;
	ros::Subscriber sub_predicted_objects;
	ros::Subscriber sub_TrafficLightSignals	;
	ros::Subscriber sub_StepSignal;
	ros::Subscriber sub_cloud_clusters;

	// Callback function for subscriber.
	void callbackGetInitPose(const geometry_msgs::PoseWithCovarianceStampedConstPtr &msg);
	void callbackGetGoalPose(const geometry_msgs::PoseStampedConstPtr &msg);
	void callbackGetPredictedObjects(const autoware_msgs::DetectedObjectArrayConstPtr& msg);
	void callbackGetTrafficLightSignals(const autoware_msgs::Signals& msg);
	void callbackGetStepForwardSignals(const geometry_msgs::TwistStampedConstPtr& msg);
	void callbackGetCloudClusters(const autoware_msgs::CloudClusterArrayConstPtr& msg);

public:
	OpenPlannerCarSimulator();

	virtual ~OpenPlannerCarSimulator();

	void MainLoop();

  void GetTransformFromTF(const std::string parent_frame, const std::string child_frame, tf::StampedTransform &transform);

  void ReadParamFromLaunchFile(PlannerHNS::CAR_BASIC_INFO& m_CarInfo,
		  PlannerHNS::ControllerParams& m_ControlParams);

  void displayFollowingInfo(const std::vector<PlannerHNS::GPSPoint>& safety_rect, PlannerHNS::WayPoint& curr_pose);
  void visualizePath(const std::vector<PlannerHNS::WayPoint>& path);

  void visualizeBehaviors();

  void SaveSimulationData();
  int LoadSimulationData(PlannerHNS::WayPoint& start_p, PlannerHNS::WayPoint& goal_p);
  void InitializeSimuCar(PlannerHNS::WayPoint start_pose);
  void PublishSpecialTF(const PlannerHNS::WayPoint& pose);
};

}

#endif  // OP_CAR_SIMULATOR