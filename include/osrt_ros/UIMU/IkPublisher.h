#ifndef IKPUBLISHER_FBK_03052023
#define IKPUBLISHER_FBK_03052023

#include "SignalProcessing.h"
#include "opensimrt_msgs/CommonTimed.h"
#include "opensimrt_msgs/PosVelAccTimed.h"
#include "opensimrt_msgs/LabelsSrv.h"
#include "ros/node_handle.h"
#include "ros/publisher.h"

class IkPublisher
{
	//filter parameters
	double cutoffFreq;
	int splineOrder, memory, delay;
	std::vector<std::string> output_labels;
	OpenSimRT::LowPassSmoothFilter * ikfilter;
	ros::NodeHandle nh{"~"};
	ros::Publisher pub; //output
	ros::Publisher pub_filtered; //output, but filtered
	bool publish_filtered = false;
	bool published_labels_at_least_once = false;
	ros::ServiceServer outLabelsSrv;
	//bool outLabels(opensimrt_msgs::LabelsSrv::Request & req, opensimrt_msgs::LabelsSrv::Response& res );
	int numSignals;
	void get_param()
	{
		nh.param<bool>("filter_output",publish_filtered, true);
		nh.param<double>("cutoff_freq", cutoffFreq, 0.0);
		nh.param<int>("memory", memory, 0);
		nh.param<int>("spline_order", splineOrder, 0);
		nh.param<int>("delay", delay, 0);
	}
	void onInit()
	{
		pub = nh.advertise<opensimrt_msgs::CommonTimed>("output", 1000);
		outLabelsSrv = nh.advertiseService("out_labels", &IkPublisher::outLabels, this);
		if(publish_filtered)
		{
			//filter
			ROS_DEBUG_STREAM("Setting up filter");
			OpenSimRT::LowPassSmoothFilter::Parameters ikFilterParam;
			ikFilterParam.numSignals = numSignals;
			ikFilterParam.memory = memory;
			ikFilterParam.delay = delay;
			ikFilterParam.cutoffFrequency = cutoffFreq;
			ikFilterParam.splineOrder = splineOrder;
			ikFilterParam.calculateDerivatives = true;
			ROS_DEBUG_STREAM("filter parameters set.");
			ikfilter = new OpenSimRT::LowPassSmoothFilter(ikFilterParam);
			// initialize filtered publisher
			pub_filtered = nh.advertise<opensimrt_msgs::PosVelAccTimed>("output_filtered", 1000);

		}


	}
	void publish(double t,  SimTK::Vector qRaw)
	{
		opensimrt_msgs::CommonTimed msg;
		std_msgs::Header h;
		h.stamp = ros::Time::now();
		h.frame_id = "subject";
		msg.header = h;

		for (double joint_angle:qRaw)
		{
			ROS_DEBUG_STREAM("some joint_angle:"<<joint_angle);
			msg.data.push_back(joint_angle);
		}

		pub.publish(msg);
		if(publish_filtered)
		{
			auto ikFiltered = ikfilter->filter({t, qRaw});
			auto q = ikFiltered.x;
			auto qDot = ikFiltered.xDot;
			auto qDDot = ikFiltered.xDDot;
			ROS_DEBUG_STREAM("Filter ran ok");
			if (!ikFiltered.isValid) {
				ROS_DEBUG_STREAM("filter results are NOT valid");
				return; }
			ROS_DEBUG_STREAM("Filter results are valid");
			opensimrt_msgs::PosVelAccTimed msg_filtered;
			msg_filtered.header = h;
			msg_filtered.time = ikFiltered.t;
			//for loop to fill the data appropriately:
			for (int i=0;i<q.size();i++)
			{
				msg_filtered.d0_data.push_back(q[i]);
				msg_filtered.d1_data.push_back(qDot[i]);
				msg_filtered.d2_data.push_back(qDDot[i]);
			}

			pub_filtered.publish(msg_filtered);
			// visualize filtered!


		}
	}
	bool outLabels(opensimrt_msgs::LabelsSrv::Request & req, opensimrt_msgs::LabelsSrv::Response& res )
	{
		if (output_labels.size() == 0)
			ROS_ERROR_STREAM("output_labels variable not set!!!! Your client will not be able to know what is what so this will possibly crash.");
		res.data = output_labels;
		published_labels_at_least_once = true;
		ROS_INFO_STREAM("CALLED LABELS SRV");
		return true;
	}



};

#endif
