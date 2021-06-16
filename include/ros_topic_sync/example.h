#pragma once

#include "ros_topic_sync/advanced_sync_policy.h"

class ExampleSync
{
public:
    typedef ros_topic_sync::AdvancedSyncPolicy<
        geometry_msgs::PointStamped,
        geometry_msgs::QuaternionStamped,
        geometry_msgs::PointStamped,
        std_msgs::Float32
    > ExampleSyncPolicy;

    ExampleSync()
    {
        auto sub0 = std::make_shared<message_filters::Subscriber<geometry_msgs::PointStamped>>(
            nh, "in/Position", queue_size
        );
        auto sub1 = std::make_shared<message_filters::Subscriber<geometry_msgs::QuaternionStamped>>(
            nh, "in/Orientation", queue_size
        );
        auto sub2 = std::make_shared<message_filters::Subscriber<geometry_msgs::PointStamped>>(
            nh, "in/TargetPosition", queue_size
        );
        auto sub3 = std::make_shared<message_filters::Subscriber<std_msgs::Float32>>(
            nh, "in/TargetOrientation", queue_size
        );
        auto sync = std::make_shared<message_filters::Synchronizer<ExampleSyncPolicy>>(
            ExampleSyncPolicy(&nh),
            *auto sub0,
            *auto sub1,
            *auto sub2,
            *auto sub3
        );
        sync->config[2].optional = true;
        sync->config[3].optional = true;
        sync->config[2].trigger = true;
        sync->config[3].trigger = true;
        sync->registerCallback(
            boost::bind(
                &ExampleSync::callback, 
                this, 
                _1, _2, _3, _4
            )
        );
    }

    void callback(
        const geometry_msgs::PointStamped::ConstPtr msg0,
        const geometry_msgs::QuaternionStamped::ConstPtr msg1,
        const geometry_msgs::PointStamped::ConstPtr msg2,
        const std_msgs::Float32::ConstPtr msg3 
    )
    {}


} // namespace ros_topic_sync
