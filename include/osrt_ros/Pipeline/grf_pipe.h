#ifndef PIPELINE_GRFM_HEADER_FBK_01062022
#define PIPELINE_GRFM_HEADER_FBK_01062022

#include "opensimrt_msgs/CommonTimed.h"
#include "opensimrt_msgs/Events.h"
#include "opensimrt_msgs/PosVelAccTimed.h"
#include "experimental/GRFMPrediction.h"
#include "SignalProcessing.h"
#include "Visualization.h"
#include <Common/TimeSeriesTable.h>
#include "osrt_ros/Pipeline/wrench_publisher.h"
#include "ros/service_server.h"
#include "std_srvs/Empty.h"
#include "Ros/include/common_node.h"

namespace Pipeline
{
	class Grf:public Ros::CommonNode
	{
		public:
			//TimeSeriesTable qTable;
			//int simulationLoops;
			//int loopCounter;
			OpenSimRT::GaitPhaseDetector* detector;
			OpenSimRT::GRFMPrediction* grfm;
			double sumDelayMS;
			int sumDelayMSCounter;
			//int i;
			OpenSimRT::InverseDynamics* id;
			OpenSimRT::BasicModelVisualizer* visualizer;
			OpenSimRT::ForceDecorator* rightGRFDecorator;
			OpenSimRT::ForceDecorator* leftGRFDecorator;
			OpenSim::TimeSeriesTable* grfRightLogger;
			OpenSim::TimeSeriesTable* grfLeftLogger;
			OpenSim::TimeSeriesTable* tauLogger;
			SimTK::Vec3 grfOrigin;
			int counter;
			double previousTime, previousTimeDifference;
			WrenchPub w_foot_r, w_foot_l;
			std::string modelFile;
			Grf();
			~Grf();
			void onInit();
			virtual void get_params();
			void run(double t, SimTK::Vector q,SimTK::Vector qDot, SimTK::Vector qDDot, std_msgs::Header h, opensimrt_msgs::Events e);
			void callback(const opensimrt_msgs::CommonTimedConstPtr& message) ;
			void callback_filtered(const opensimrt_msgs::PosVelAccTimedConstPtr& message) ;
			void finish(); 
			bool see(std_srvs::Empty::Request &req, std_srvs::Empty::Response &res);
			void write_();
			OpenSimRT::LowPassSmoothFilter* filter;
			std::string subjectDir;
			
			ros::Publisher sync_output, sync_output_filtered;

			//virtual void detectorFunction();	
	};
}
#endif

