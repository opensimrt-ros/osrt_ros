#ifndef EXTERNAL_HEADING_H_29032024
#define EXTERNAL_HEADING_H_29032024

#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/Quaternion.h>
#include "geometry_msgs/Transform.h"
#include "tf/LinearMath/Vector3.h"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include <tf2_ros/static_transform_broadcaster.h>
#include "std_srvs/Empty.h" //the warning is wrong
#include "visualization_msgs/Marker.h"
//#include "std_srvs/EmptyRequest.h"
//#include "std_srvs/EmptyResponse.h"

const std::string red("\033[0;31m");
const std::string green("\033[1;32m");
const std::string yellow("\033[1;33m");
const std::string cyan("\033[0;36m");
const std::string magenta("\033[0;35m");
const std::string reset("\033[0m");

class ExternalHeading
{
	public:
		ros::NodeHandle n;
	std::vector<visualization_msgs::Marker> debug_markers;

		std::string imu_default_frame_name, imu_heading_axis, imu_base_measured_frame_name, opensim_base_default_frame_name, added_heading_frame_name, parent_frame_name;
		tf2_ros::StaticTransformBroadcaster br;
		tf2_ros::Buffer tfBuffer;
		tf2_ros::TransformListener tfListener;
		geometry_msgs::Point origin,debug_point ;
		geometry_msgs::Vector3 heading, opensim_heading;
		ExternalHeading();
		bool calibrate_srv(std_srvs::EmptyRequest & req, std_srvs::EmptyResponse & res);

		double calculate_angle(geometry_msgs::Transform default_measure_frame, geometry_msgs::Transform measured_frame, geometry_msgs::Vector3 imu_heading_axis_vector);

		geometry_msgs::PoseStamped calibrate();

		visualization_msgs::Marker getArrowForVector(std::string name, geometry_msgs::Vector3, geometry_msgs::Point origin_);
		visualization_msgs::Marker getArrowForVector(std::string name, geometry_msgs::Vector3);

};
#endif // EXTERNAL_HEADING_H_29032024
