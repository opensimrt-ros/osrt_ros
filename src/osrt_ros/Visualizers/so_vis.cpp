#include "osrt_ros/Visualizers/so_vis.h"
#include "opensimrt_msgs/MultiMessage.h"
#include <SimTKcommon/internal/BigMatrix.h>
/*void Visualizers::SoVis::before_vis()
{
	//TODO:
	ROS_ERROR_STREAM("not implemented");
}
*/
void Visualizers::SoVis::callback(const opensimrt_msgs::DualConstPtr &message)
{
	ROS_INFO_STREAM("callback sovis reached received message:" << message);
	//initialize q
	SimTK::Vector q(message->q.data.size()), soOutput_am(message->tau.data.size()); //TODO: its not tau the name of the message is unfortunate change it, so that this reads better
	for (int i=0;i<message->q.data.size(); i++)
	{
		q[i] = message->q.data[i];
	}
	for (int i=0;i<message->tau.data.size(); i++)
	{
		soOutput_am[i] = message->tau.data[i];
	}
	try {

		visualizer->update(q, soOutput_am);
	}
	catch (std::exception& e)
	{
		ROS_ERROR_STREAM("Error in visualizer. cannot show data!!!!!" <<std::endl << e.what());
	}
}
void Visualizers::SoVis::callback_multi(const opensimrt_msgs::MultiMessageConstPtr &message)
{
	ROS_DEBUG_STREAM("callback sovis multi reached received message:" << message);
	//initialize q
	SimTK::Vector q(message->ik.data.size()), soOutput_am(message->other[0].data.size()); //TODO: its not tau the name of the message is unfortunate change it, so that this reads better
	for (int i=0;i<message->ik.data.size(); i++)
	{
		q[i] = message->ik.data[i];
	}
	for (int i=0;i<message->other[0].data.size(); i++)
	{
		soOutput_am[i] = message->other[0].data[i];
	}
	try {

		visualizer->update(q, soOutput_am);
	}
	catch (std::exception& e)
	{
		ROS_ERROR_STREAM("Error in visualizer. cannot show data!!!!!" <<std::endl << e.what());
	}
}
void Visualizers::SoVis::callback_filtered(const opensimrt_msgs::DualPosConstPtr &message)
{
	ROS_INFO_STREAM("callback sovis filtered reached received message:" << message);
	//initialize q
	SimTK::Vector q(message->qqq.d0_data.size()), soOutput_am(message->tau.data.size()); //TODO: its not tau the name of the message is unfortunate change it, so that this reads better
	for (int i=0;i<message->qqq.d0_data.size(); i++)
	{
		q[i] = message->qqq.d0_data[i];
	}
	for (int i=0;i<message->tau.data.size(); i++)
	{
		soOutput_am[i] = message->tau.data[i];
	}
	try {

		visualizer->update(q, soOutput_am);
	}
	catch (std::exception& e)
	{
		ROS_ERROR_STREAM("Error in visualizer. cannot show data!!!!!" <<std::endl << e.what());
	}
}
