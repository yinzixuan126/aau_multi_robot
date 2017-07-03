#include "docking.h"
#include "battery_simulate.h"
#include "mock_time_manager.h"
#include <ros/ros.h>
#include <gtest/gtest.h>

double const charge_max = 200;
double const power_standing = 37;
double const power_moving = 19;
double const power_basic_computations = 10;
double const power_advanced_computations = 23;
double const power_charging = 50;
double const power_idle = 1;
double const max_linear_speed = 0.5;

enum state_t
    {
        exploring,  // the robot is computing which is the next frontier to be
                    // explored

        going_charging,  // the robot has the right to occupy a DS to recharge

        charging,  // the robot is charging at a DS

        finished,  // the robot has finished the exploration

        fully_charged,  // the robot has recently finished a charging process; notice
                        // that the robot is in this state even if it is not really
                        // fully charged (since just after a couple of seconds after
                        // the end of the recharging process the robot has already
                        // lost some battery energy, since it consumes power even
                        // when it stays still

        stuck,

        in_queue,  // the robot is in a queue, waiting for a DS to be vacant

        auctioning,  // auctioning: the robot has started an auction; notice that if
                     // the robot is aprticipating to an auction that it was not
                     // started by it, its state is not equal to auctioning!!!
                     
        auctioning_2,

        going_in_queue,  // the robot is moving near a DS to later put itself in
                         // in_queue state

        going_checking_vacancy,  // the robot is moving near a DS to check if it
                                 // vacant, so that it can occupy it and start
                                 // recharging

        checking_vacancy,  // the robot is currently checking if the DS is vacant,
                           // i.e., it is waiting information from the other robots
                           // about the state of the DS

        moving_to_frontier_before_going_charging,  // TODO hmm...

        moving_to_frontier,  // the robot has selected the next frontier to be
                             // reached, and it is moving toward it
        leaving_ds,          // the robot was recharging, but another robot stopped
        dead,
        moving_away_from_ds,
        auctioning_3
    };

// Declare a test
TEST(TestSuite, testCase1)
{
    int i = 10;
    EXPECT_NE(i, 1);
    ; //<test things here, calling EXPECT_* and/or ASSERT_* macros as needed>
}

// Declare another test
TEST(TestSuite, testCase2)
{
    MockTimeManager mtm;
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    EXPECT_EQ(bat.getChargeMax(), charge_max);
}

TEST(TestSuite, testCase3)
{
    MockTimeManager mtm;
    double time = 30;
    mtm.addTime(time);
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    bat.set_last_time();
    EXPECT_EQ(bat.last_time_secs(), time);
}

TEST(TestSuite, testCase4)
{  
    battery_simulate bat;
    double test_total_energy = charge_max * 3600;
    double total_energy = bat.getRemainingEnergy();
    EXPECT_EQ(total_energy, test_total_energy);
}

TEST(TestSuite, testCase5)
{
    MockTimeManager mtm;
    double time1 = 30, time2 = 70, time3 = 120;
    mtm.addTime(time1);
    mtm.addTime(time2);
    mtm.addTime(time3);
    
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    bat.initializeSimulationTime();
    
    EXPECT_EQ(time1, bat.last_time_secs());
    bat.compute();
    EXPECT_EQ(time3, bat.last_time_secs());
    EXPECT_EQ(40, bat.getElapsedTime());
}

TEST(TestSuite, testCase6)
{
    MockTimeManager mtm;
    mtm.addTime(0);
    mtm.addTime(30);
    double test_time = 70;
    mtm.addTime(test_time);
    mtm.addTime(100);
    
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    bat.initializeSimulationTime();
    bat.compute();

    EXPECT_EQ(test_time, bat.last_time_secs());
}

TEST(TestSuite, testCase7)
{
    MockTimeManager mtm;
    double time1 = 30, time2 = 70, time3 = 120, time4 = 200, time5 = 250;
    mtm.addTime(time1);
    mtm.addTime(time2);
    mtm.addTime(time3);
    mtm.addTime(time4);
    mtm.addTime(time5);
    
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    bat.initializeSimulationTime();
    
    ros::NodeHandle nh;
    
    ros::Publisher cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 10, true);
    geometry_msgs::Twist cmd_vel_msg;
    cmd_vel_msg.linear.x = 0;
    cmd_vel_pub.publish(cmd_vel_msg);
    
    ros::Publisher robot_pub = nh.advertise<adhoc_communication::EmRobot>("explorer/robot", 10, true);
    adhoc_communication::EmRobot robot_state_msg;
    robot_state_msg.state = exploring;
    robot_pub.publish(robot_state_msg);
    
    ros::Duration(1).sleep();
    bat.spinOnce();
    
    double total_energy, remaining_energy, test_remaining_energy;
    total_energy  = bat.getRemainingEnergy();
    bat.compute();
    remaining_energy = bat.getRemainingEnergy();
    test_remaining_energy = total_energy - (power_standing + power_basic_computations + power_advanced_computations) * (time2 - time1);
    EXPECT_EQ(time3, bat.last_time_secs());
    EXPECT_EQ(time2 - time1, bat.getElapsedTime());
    EXPECT_EQ(test_remaining_energy, remaining_energy);
    
    bat.compute();
    EXPECT_EQ(time5, bat.last_time_secs());
    EXPECT_EQ(time4 - time3, bat.getElapsedTime());
    test_remaining_energy -= (power_standing + power_basic_computations + power_advanced_computations) * (time4 - time3);
    remaining_energy = bat.getRemainingEnergy();
    EXPECT_EQ(test_remaining_energy, remaining_energy);
}

TEST(TestSuite, testCase8)
{
    MockTimeManager mtm;
    double time1 = 30, time2 = 70, time3 = 120, time4 = 200, time5 = 250;
    mtm.addTime(time1);
    mtm.addTime(time2);
    mtm.addTime(time3);
    mtm.addTime(time4);
    mtm.addTime(time5);
    
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    bat.initializeSimulationTime();
    
    ros::NodeHandle nh;
    
    ros::Publisher cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 10, true);
    geometry_msgs::Twist cmd_vel_msg;
    cmd_vel_msg.linear.x = 0;
    cmd_vel_pub.publish(cmd_vel_msg);
    
    ros::Publisher robot_pub = nh.advertise<adhoc_communication::EmRobot>("explorer/robot", 10, true);
    adhoc_communication::EmRobot robot_state_msg;
    robot_state_msg.state = moving_to_frontier;
    robot_pub.publish(robot_state_msg);
    
    ros::Duration(1).sleep(); // necessary, or the messages are not received in time by the battery manager
    bat.spinOnce();
    
    double total_energy, remaining_energy, test_remaining_energy;
    total_energy  = bat.getRemainingEnergy();
    bat.compute();
    remaining_energy = bat.getRemainingEnergy();
    test_remaining_energy = total_energy - (power_standing + power_basic_computations) * (time2 - time1);
    EXPECT_EQ(time3, bat.last_time_secs());
    EXPECT_EQ(time2 - time1, bat.getElapsedTime());
    EXPECT_EQ(test_remaining_energy, remaining_energy);
    
    cmd_vel_msg.linear.x = 10;
    cmd_vel_pub.publish(cmd_vel_msg);
    
    ros::Duration(1).sleep();
    bat.spinOnce();
    
    bat.compute();
    EXPECT_EQ(time5, bat.last_time_secs());
    EXPECT_EQ(time4 - time3, bat.getElapsedTime());
    test_remaining_energy -= (power_moving * max_linear_speed + power_standing + power_basic_computations) * (time4 - time3);
    remaining_energy = bat.getRemainingEnergy();
    EXPECT_EQ(test_remaining_energy, remaining_energy);
}

TEST(TestSuite, testCase9)
{
    MockTimeManager mtm;
    double time1 = 0, time2 = 500, time3 = 800, time4 = 1000, time5 = 1250;
    mtm.addTime(time1);
    mtm.addTime(time2);
    mtm.addTime(time3);
    mtm.addTime(time4);
    mtm.addTime(time5);
    
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    bat.initializeSimulationTime();
    
    ros::NodeHandle nh;
    
    ros::Publisher cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 10, true);
    geometry_msgs::Twist cmd_vel_msg;
    cmd_vel_msg.linear.x = 10;
    cmd_vel_pub.publish(cmd_vel_msg);
    
    ros::Publisher robot_pub = nh.advertise<adhoc_communication::EmRobot>("explorer/robot", 10, true);
    adhoc_communication::EmRobot robot_state_msg;
    robot_state_msg.state = exploring;
    robot_pub.publish(robot_state_msg);
    
    ros::Duration(1).sleep(); // necessary, or the messages are not received in time by the battery manager
    bat.spinOnce();
    
    double total_energy, remaining_energy, test_remaining_energy;
    total_energy  = bat.getRemainingEnergy();
    bat.compute();
    remaining_energy = bat.getRemainingEnergy();
    test_remaining_energy = total_energy - (power_moving * max_linear_speed + power_standing + power_basic_computations + power_advanced_computations) * (time2 - time1);
    EXPECT_EQ(time3, bat.last_time_secs());
    EXPECT_EQ(time2 - time1, bat.getElapsedTime());
    EXPECT_EQ(test_remaining_energy, remaining_energy);
    
    robot_state_msg.state = charging;
    robot_pub.publish(robot_state_msg);
    
    ros::Duration(1).sleep();
    bat.spinOnce();
    
    bat.compute();
    EXPECT_EQ(time5, bat.last_time_secs());
    EXPECT_EQ(time4 - time3, bat.getElapsedTime());
    test_remaining_energy += power_charging * (time4 - time3);
    remaining_energy = bat.getRemainingEnergy();
    EXPECT_EQ(test_remaining_energy, remaining_energy);
}

TEST(TestSuite, testCase10)
{
    MockTimeManager mtm;
    double time1 = 0, time2 = 1000, time3 = 1250;
    mtm.addTime(time1);
    mtm.addTime(time2);
    mtm.addTime(time3);
    
    battery_simulate bat;
    bat.setTimeManager(&mtm);
    bat.initializeSimulationTime();
    
    ros::NodeHandle nh;
    
    ros::Publisher cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 10, true);
    geometry_msgs::Twist cmd_vel_msg;
    cmd_vel_msg.linear.x = 10;
    cmd_vel_pub.publish(cmd_vel_msg);
    
    ros::Publisher robot_pub = nh.advertise<adhoc_communication::EmRobot>("explorer/robot", 10, true);
    adhoc_communication::EmRobot robot_state_msg;
    robot_state_msg.state = in_queue;
    robot_pub.publish(robot_state_msg);
    
    ros::Duration(1).sleep(); // necessary, or the messages are not received in time by the battery manager
    bat.spinOnce();
    
    double total_energy, remaining_energy, test_remaining_energy;
    total_energy  = bat.getRemainingEnergy();
    bat.compute();
    remaining_energy = bat.getRemainingEnergy();
    test_remaining_energy = total_energy - (power_idle) * (time2 - time1);
    EXPECT_EQ(time3, bat.last_time_secs());
    EXPECT_EQ(time2 - time1, bat.getElapsedTime());
    EXPECT_EQ(test_remaining_energy, remaining_energy);
}

int main(int argc, char **argv){
  ros::init(argc, argv, "energy_mgmt");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
