#pragma once

#include "message_filters/synchronizer.h"
#include "message_filters/connection.h"
#include "message_filters/null_types.h"
#include "message_filters/signal9.h"

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/bind.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/noncopyable.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/vector.hpp>

#include <ros/ros.h>
#include <ros/assert.h>
#include <ros/message_traits.h>
#include <ros/message_event.h>

#include <deque>
#include <vector>
#include <string>

namespace ros_topic_sync {

namespace mpl = boost::mpl;


template<
    typename M0, typename M1, typename M2 = message_filters::NullType, typename M3 = message_filters::NullType, typename M4 = message_filters::NullType,
    typename M5 = message_filters::NullType, typename M6 = message_filters::NullType, typename M7 = message_filters::NullType, typename M8 = message_filters::NullType
>
struct ResamplingSyncPolicy : public message_filters::PolicyBase<M0, M1, M2, M3, M4, M5, M6, M7, M8>
{
    typedef message_filters::Synchronizer<ResamplingSyncPolicy> Sync;
    typedef message_filters::PolicyBase<M0, M1, M2, M3, M4, M5, M6, M7, M8> Super;
    typedef typename Super::Messages Messages;
    typedef typename Super::Signal Signal;
    typedef typename Super::Events Events;
    typedef typename Super::RealTypeCount RealTypeCount;
    typedef typename Super::M0Event M0Event;
    typedef typename Super::M1Event M1Event;
    typedef typename Super::M2Event M2Event;
    typedef typename Super::M3Event M3Event;
    typedef typename Super::M4Event M4Event;
    typedef typename Super::M5Event M5Event;
    typedef typename Super::M6Event M6Event;
    typedef typename Super::M7Event M7Event;
    typedef typename Super::M8Event M8Event;
    typedef boost::tuple<M0Event, M1Event, M2Event, M3Event, M4Event, M5Event, M6Event, M7Event, M8Event> Tuple;

    ResamplingSyncPolicy(ros::NodeHandle* nh, double resampleInterval)
        : m_parent(0)
        , m_nh(nh)
        , m_resampleInterval(resampleInterval)
    {
        m_resample_timer = m_nh->createSteadyTimer(
            ros::WallDuration(m_resampleInterval), 
            &ResamplingSyncPolicy::resample, 
            this
        );
    }

    ResamplingSyncPolicy(const ResamplingSyncPolicy& e)
    {
        *this = e;
    }

    ResamplingSyncPolicy& operator=(const ResamplingSyncPolicy& rhs)
    {
        m_parent = rhs.m_parent;
        m_nh = rhs.m_nh;
        m_resampleInterval = rhs.m_resampleInterval;
        m_resample_timer = m_nh->createSteadyTimer(
            ros::WallDuration(m_resampleInterval), 
            &ResamplingSyncPolicy::resample, 
            this
        );

        return *this;
    }

    void initParent(Sync* parent)
    {
        m_parent = parent;
    }

    template<int i>
    void add(const typename mpl::at_c<Events, i>::type& evt)
    {
        ROS_ASSERT(m_parent);
        // ROS_INFO("ResamplingSyncPolicy::add"); 

        boost::mutex::scoped_lock lock(m_mutex);

        boost::get<i>(m_tuple) = evt;

    }

    template<class C>
    message_filters::Connection registerDropCallback(const C& callback)
    {
        return m_drop_signal.template addCallback(callback);
    }

    template<class C>
    message_filters::Connection registerDropCallback(C& callback)
    {
        return m_drop_signal.template addCallback(callback);
    }

    template<class C, typename T>
    message_filters::Connection registerDropCallback(const C& callback, T* t)
    {
        return m_drop_signal.template addCallback(callback, t);
    }

    template<class C, typename T>
    message_filters::Connection registerDropCallback(C& callback, T* t)
    {
        return m_drop_signal.template addCallback(callback, t);
    }

private:

    void resample(const ros::SteadyTimerEvent&)
    {
        boost::mutex::scoped_lock lock(m_mutex);
        // ROS_INFO("RESAMPLE");
        checkTuple(m_tuple);
    }

    // assumes m_mutex_ is already locked
    void checkTuple(Tuple& t)
    {
        namespace mt = ros::message_traits;

        bool full = true;
        full = full && (bool)boost::get<0>(t).getMessage();
        full = full && (bool)boost::get<1>(t).getMessage();
        full = full && (RealTypeCount::value > 2 ? (bool)boost::get<2>(t).getMessage() : true);
        full = full && (RealTypeCount::value > 3 ? (bool)boost::get<3>(t).getMessage() : true);
        full = full && (RealTypeCount::value > 4 ? (bool)boost::get<4>(t).getMessage() : true);
        full = full && (RealTypeCount::value > 5 ? (bool)boost::get<5>(t).getMessage() : true);
        full = full && (RealTypeCount::value > 6 ? (bool)boost::get<6>(t).getMessage() : true);
        full = full && (RealTypeCount::value > 7 ? (bool)boost::get<7>(t).getMessage() : true);
        full = full && (RealTypeCount::value > 8 ? (bool)boost::get<8>(t).getMessage() : true);

        if (full)
        {
            m_parent->signal(
                boost::get<0>(t), boost::get<1>(t), boost::get<2>(t),
                boost::get<3>(t), boost::get<4>(t), boost::get<5>(t),
                boost::get<6>(t), boost::get<7>(t), boost::get<8>(t)
            );

        }
    }

private:
    Sync* m_parent;
    ros::NodeHandle* m_nh;
    Tuple m_tuple;
    ros::SteadyTimer m_resample_timer;
    double m_resampleInterval;

    Signal m_drop_signal;

    boost::mutex m_mutex;
};

} // namespace ros_topic_sync
