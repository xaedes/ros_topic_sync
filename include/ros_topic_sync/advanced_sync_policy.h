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
    typename M0, 
    typename M1, 
    typename M2 = message_filters::NullType, 
    typename M3 = message_filters::NullType, 
    typename M4 = message_filters::NullType,
    typename M5 = message_filters::NullType, 
    typename M6 = message_filters::NullType, 
    typename M7 = message_filters::NullType, 
    typename M8 = message_filters::NullType
>
struct AdvancedSyncPolicy : public message_filters::PolicyBase<M0, M1, M2, M3, M4, M5, M6, M7, M8>
{
    typedef message_filters::Synchronizer<AdvancedSyncPolicy> Sync;
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

    static const uint8_t MAX_MESSAGES = 9;

    struct Config
    {
        bool trigger;
        bool delay;
        bool optional;
        bool clear;
        Config()
            : trigger(false)  // should arrival of new message trigger callback?
            , delay(false)    // use second last message instead of last message to give to callback
            , optional(false) // is message on topic necessary to trigger callback or is it optional?
            , clear(false)    // clear the last value after callback? together with optional will callback with None
        {}
    };


    Config m_config[MAX_MESSAGES];
    bool m_cleared[MAX_MESSAGES];

    AdvancedSyncPolicy(ros::NodeHandle* nh)
        : m_parent(0)
        , m_nh(nh)
    {
        for (int i = 0; i < MAX_MESSAGES; ++i)
        {
            m_config[i] = Config();
            m_cleared[i] = false;
        }
        m_neverSignaled = true;
    }

    AdvancedSyncPolicy(const AdvancedSyncPolicy& e)
    {
        *this = e;
    }

    AdvancedSyncPolicy& operator=(const AdvancedSyncPolicy& rhs)
    {
        m_parent = rhs.m_parent;
        m_nh = rhs.m_nh;
        m_neverSignaled = rhs.m_neverSignaled;
        for (int i = 0; i < MAX_MESSAGES; ++i)
        {
            m_config[i] = rhs.m_config[i];
            m_cleared[i] = rhs.m_cleared[i];
        }

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
        // ROS_INFO("AdvancedSyncPolicy::add");

        boost::mutex::scoped_lock lock(m_mutex);
        
        // ROS_INFO("i                   \t %d", i);
        // ROS_INFO("m_config[i].trigger \t %d", m_config[i].trigger ? 0 : 1);
        // ROS_INFO("m_config[i].delay   \t %d", m_config[i].delay);

        if (m_config[i].delay)
        {
            // ROS_INFO("delay message");
            if(contains<i>(m_newest))
            {
                // ROS_INFO("use last message");
                boost::get<i>(m_tuple) = boost::get<i>(m_newest);
                m_cleared[i] = false;
            }
            boost::get<i>(m_newest) = evt;
        }
        else
        {
            boost::get<i>(m_tuple) = evt;
            m_cleared[i] = false;
            // ROS_INFO("set message(%d)", i);
        }

        if(m_config[i].trigger || m_neverSignaled)
        {
            // ROS_INFO("checkTuple");
            checkTuple(m_tuple);
        }
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

    // assumes m_mutex_ is already locked
    void checkTuple(Tuple& t)
    {
        namespace mt = ros::message_traits;

        if (isComplete(t))
        {
            // ROS_INFO("signal");
            m_parent->signal(
                boost::get<0>(contains<0>(t) ? t : m_emptyTuple) , 
                boost::get<1>(contains<1>(t) ? t : m_emptyTuple) , 
                boost::get<2>(contains<2>(t) ? t : m_emptyTuple) ,
                boost::get<3>(contains<3>(t) ? t : m_emptyTuple) , 
                boost::get<4>(contains<4>(t) ? t : m_emptyTuple) , 
                boost::get<5>(contains<5>(t) ? t : m_emptyTuple) ,
                boost::get<6>(contains<6>(t) ? t : m_emptyTuple) , 
                boost::get<7>(contains<7>(t) ? t : m_emptyTuple) , 
                boost::get<8>(contains<8>(t) ? t : m_emptyTuple) 
            );
            processMessageClearing<0>(t);
            processMessageClearing<1>(t);
            processMessageClearing<2>(t);
            processMessageClearing<3>(t);
            processMessageClearing<4>(t);
            processMessageClearing<5>(t);
            processMessageClearing<6>(t);
            processMessageClearing<7>(t);
            processMessageClearing<8>(t);
            m_neverSignaled = false;
        }
    }

    template<int I>
    bool processMessageClearing(Tuple& t)
    {
        if(m_config[I].clear && contains<I>(t))
        {
            m_cleared[I] = true;
            assert(!contains<I>(t));
        }
    }


    bool isComplete(Tuple& t)
    {
        return((m_config[0].optional || contains<0>(t))
            && (m_config[1].optional || contains<1>(t))
            && (m_config[2].optional || contains<2>(t))
            && (m_config[3].optional || contains<3>(t))
            && (m_config[4].optional || contains<4>(t))
            && (m_config[5].optional || contains<5>(t))
            && (m_config[6].optional || contains<6>(t))
            && (m_config[7].optional || contains<7>(t))
            && (m_config[8].optional || contains<8>(t))
        );
    }

    template<int I>
    bool contains(Tuple& t)
    {
        if(m_config[I].clear && m_cleared[I]) return false;
        switch(I)
        {
        case 0:
            return (bool)boost::get<0>(t).getMessage();
        case 1:
            return (bool)boost::get<1>(t).getMessage();
        default:
            return (RealTypeCount::value > I) ? (bool)boost::get<I>(t).getMessage() : true;
        }
    }

private:
    Sync* m_parent;
    ros::NodeHandle* m_nh;
    Tuple m_tuple;
    Tuple m_emptyTuple;

    Signal m_drop_signal;

    boost::mutex m_mutex;

    Tuple m_newest;

    bool m_neverSignaled;

};

} // namespace ros_topic_sync 
