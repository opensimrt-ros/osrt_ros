#ifndef VISUALIZER_GRFM_HEADER_FBK_27052023
#define VISUALIZER_GRFM_HEADER_FBK_27052023

#include "experimental/GRFMPrediction.h"
#include "opensimrt_msgs/Dual.h"
#include "opensimrt_msgs/DualPos.h"
#include "osrt_ros/Visualizers/dualsink_vis.h"

namespace Visualizers
{
	class GrfVis:public Visualizers::DualSinkVis
	{
		public:
			OpenSimRT::ForceDecorator* rightGRFDecorator;
			OpenSimRT::ForceDecorator* leftGRFDecorator;
			void callback(const opensimrt_msgs::DualConstPtr& message);
			void callback_filtered(const opensimrt_msgs::DualPosConstPtr& message);
			void after_vis();

			std::string right_body_name;
			std::string left_body_name;
			//void after_callback(); //this doesnt work because we need to pass information for the things after the callback...
			//
			void before_vis();
	};
}
#endif

