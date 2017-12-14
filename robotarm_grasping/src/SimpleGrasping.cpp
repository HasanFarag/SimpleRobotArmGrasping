#include "robotarm_grasping/SimpleGrasping.h"

SimpleGrasping::SimpleGrasping(ros::NodeHandle n_, float length, float breadth)
    : it_(n_), armgroup("arm"), grippergroup("gripper"), vMng_(length, breadth)
{
    this->nh_ = n_;
    namespace rvt = rviz_visual_tools;
    // visual_tools("odom_combined");
	//visual_tools.deleteAllMarkers();

    // Subscribe to input video feed and publish output video feed
    image_sub_ = it_.subscribe("/robot_arm/camera1/robot_arm/image_raw", 1,
      &SimpleGrasping::imageCb, this);

    // this->display_publisher = nh_.advertise<moveit_msgs::DisplayTrajectory>("/move_group/display_planned_path", 1, true);

     try {
         this->tf_camera_to_robot.waitForTransform("/base_link", "/camera_link", ros::Time(0), ros::Duration(50.0) );
     }
   catch (tf::TransformException &ex) {
            ROS_ERROR("[adventure_slam]: (wait) %s", ex.what());
            ros::Duration(1.0).sleep();
		} 

    try {
        this->tf_camera_to_robot.lookupTransform("/base_link", "/camera_link", ros::Time(0), (this->camera_to_robot_));
      }
 
    catch (tf::TransformException &ex) {
      ROS_ERROR("[adventure_slam]: (lookup) %s", ex.what());
	} 

    obj_found = false;
}

void SimpleGrasping::imageCb(const sensor_msgs::ImageConstPtr& msg)
{
	if (!obj_found) {
	    ROS_INFO("Start Processing the Image");
	    try {
	      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
	    }
	    catch (cv_bridge::Exception& e){
	      ROS_ERROR("cv_bridge exception: %s", e.what());
	      return;
		}

	    ROS_INFO("Image Message Received");
	    float obj_x, obj_y;
	    vMng_.get2DLocation(cv_ptr->image, obj_x, obj_y);

        // Temporary Debugging
	    std::cout<< " X-Co-ordinate in Camera Frame :" << obj_x << std::endl;
	    std::cout<< " Y-Co-ordinate in Camera Frame :" << obj_y << std::endl;

	    obj_camera_frame.setY(-obj_x);
	    obj_camera_frame.setZ(-obj_y);
	    obj_camera_frame.setX(4.7);
        
	    obj_robot_frame = camera_to_robot_ * obj_camera_frame;
	    obj_found = true;

	    std::cout<< " X-Co-ordinate in Robot Frame :" << obj_robot_frame.getX() << std::endl;
	    std::cout<< " Y-Co-ordinate in Robot Frame :" << obj_robot_frame.getY() << std::endl;
	    std::cout<< " Z-Co-ordinate in Robot Frame :" << obj_robot_frame.getZ() << std::endl;
	    // cv::waitKey(0);
	}
}

void SimpleGrasping::attainPosition() {
  ROS_INFO("The attain position function called");
  ros::AsyncSpinner spinner(1);
  spinner.start();
  // sleep(2.0);

  // Raw pointers are frequently used to refer to the planning group for improved performance.
   const robot_state::JointModelGroup *joint_model_group =
	armgroup.getCurrentState()->getJointModelGroup("arm"); 

  namespace rvt = rviz_visual_tools;
  moveit_visual_tools::MoveItVisualTools visual_tools("base_link_2");
  visual_tools.deleteAllMarkers();

  // We can print the name of the reference frame for this robot.
  ROS_INFO("Reference frame: %s", armgroup.getPlanningFrame().c_str());
  
  // We can also print the name of the end-effector link for this group.
  ROS_INFO("Reference frame: %s", armgroup.getEndEffectorLink().c_str());

  ROS_INFO("Group names: %s", 	armgroup.getName().c_str());

  // For getting the pose
  geometry_msgs::PoseStamped currPose = armgroup.getCurrentPose();
  std::cout<<armgroup.getCurrentPose();

  geometry_msgs::Pose target_pose1;
  target_pose1.orientation = currPose.pose.orientation;
  //target_pose1.position.x = 0.4;
  //target_pose1.position.y = 0.04;
  //target_pose1.position.z = 0.32;

   // Starting Postion before picking
    target_pose1.position.x = 0.45;
    target_pose1.position.y = 0.09;
    target_pose1.position.z = 0.37;
  armgroup.setPoseTarget(target_pose1);

  moveit::planning_interface::MoveGroupInterface::Plan my_plan;

  armgroup.plan(my_plan);

  /*ROS_INFO_NAMED("tutorial", "Visualizing plan 1 (pose goal) %s", success ? "" : "FAILED");

  ROS_INFO_NAMED("tutorial", "Visualizing plan 1 as trajectory line");
  visual_tools.publishAxisLabeled(target_pose1, "pose1");
  // visual_tools.publishText(text_pose, "Pose Goal", rvt::WHITE, rvt::XLARGE);
  visual_tools.publishTrajectoryLine(my_plan.trajectory_, joint_model_group);
  visual_tools.trigger();
  visual_tools.prompt("next step");*/

  armgroup.move();
  sleep(5.0);
}

void SimpleGrasping::attainObject() {
  ROS_INFO("The attain Object function called");
  ros::AsyncSpinner spinner(1);
  spinner.start();
  // sleep(2.0);

  // Raw pointers are frequently used to refer to the planning group for improved performance.
   const robot_state::JointModelGroup *joint_model_group =
	armgroup.getCurrentState()->getJointModelGroup("arm"); 

  namespace rvt = rviz_visual_tools;
  moveit_visual_tools::MoveItVisualTools visual_tools("base_link_2");
  visual_tools.deleteAllMarkers();

  // We can print the name of the reference frame for this robot.
  ROS_INFO("Reference frame: %s", armgroup.getPlanningFrame().c_str());
  
  // We can also print the name of the end-effector link for this group.
  ROS_INFO("Reference frame: %s", armgroup.getEndEffectorLink().c_str());

  ROS_INFO("Group names: %s", 	armgroup.getName().c_str());

  // For getting the pose
  geometry_msgs::PoseStamped currPose = armgroup.getCurrentPose();
  // std::cout<<armgroup.getCurrentPose();

  geometry_msgs::Pose target_pose1;
  target_pose1.orientation = currPose.pose.orientation;
  //target_pose1.position.x = 0.4;
  //target_pose1.position.y = 0.04;
  //target_pose1.position.z = 0.32;

   // Starting Postion before picking
   target_pose1.position.x = obj_robot_frame.getX();
   target_pose1.position.y = obj_robot_frame.getY();
   target_pose1.position.z = obj_robot_frame.getZ();
   armgroup.setPoseTarget(target_pose1);

  moveit::planning_interface::MoveGroupInterface::Plan my_plan;

  armgroup.plan(my_plan);

  /*ROS_INFO_NAMED("tutorial", "Visualizing plan 1 (pose goal) %s", success ? "" : "FAILED");

  ROS_INFO_NAMED("tutorial", "Visualizing plan 1 as trajectory line");
  visual_tools.publishAxisLabeled(target_pose1, "pose1");
  // visual_tools.publishText(text_pose, "Pose Goal", rvt::WHITE, rvt::XLARGE);
  visual_tools.publishTrajectoryLine(my_plan.trajectory_, joint_model_group);
  visual_tools.trigger();
  visual_tools.prompt("next step");*/

  armgroup.move();
  sleep(5.0);

  // Slide down the Object
  currPose = armgroup.getCurrentPose();
  std::cout<<armgroup.getCurrentPose();

  target_pose1.orientation = currPose.pose.orientation;
  target_pose1.position = currPose.pose.position;

  target_pose1.position.z = target_pose1.position.z - 0.03;
  armgroup.setPoseTarget(target_pose1);

  armgroup.plan(my_plan);

  armgroup.move();
  sleep(5.0);

}

void SimpleGrasping::grasp() {
  ROS_INFO("The Grasping function called");
  ros::AsyncSpinner spinner(1);
  spinner.start();

  ros::WallDuration(1.0).sleep();   
  grippergroup.setNamedTarget("Close");
  grippergroup.move();
  sleep(5.0);
}

void SimpleGrasping::lift() {
  ROS_INFO("The lift function called");
  ros::AsyncSpinner spinner(1);
  spinner.start();
  // sleep(2.0);

  // For getting the pose
  geometry_msgs::PoseStamped currPose = armgroup.getCurrentPose();

  geometry_msgs::Pose target_pose1;
  target_pose1.orientation = currPose.pose.orientation;
  target_pose1.position = currPose.pose.position;

   // Starting Postion before picking
   target_pose1.position.z = target_pose1.position.z + 0.06;
   armgroup.setPoseTarget(target_pose1);

  moveit::planning_interface::MoveGroupInterface::Plan my_plan;

  armgroup.plan(my_plan);

  armgroup.move();
  sleep(5.0);
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "simple_grasping");
  float length = 1;
  float breadth = 0.6;

  ros::NodeHandle n;
  SimpleGrasping simGrasp(n); 
  ros::spinOnce();
  simGrasp.attainPosition();
  simGrasp.attainObject();
  simGrasp.grasp();
  simGrasp.lift();
  return 0;
}