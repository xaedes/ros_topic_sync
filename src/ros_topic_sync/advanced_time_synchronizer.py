
import rospy
import message_filters
import threading

class AdvancedTimeSynchronizer(message_filters.SimpleFilter):

    class TopicConfig:
        def __init__(self, trigger=False, delay=False, optional=False, clear=False):
            self.trigger  = trigger  # should arrival of new message trigger callback?
            self.delay    = delay    # use second last message instead of last message to give to callback
            self.optional = optional # is message on topic necessary to trigger callback or is it optional?
            self.clear    = clear    # clear the last value after callback? together with optional will callback with None
        def __repr__(self):
            return str(dict({
                "trigger": self.trigger, 
                "delay": self.delay,
                "optional": self.optional, 
                "clear": self.clear}))

    class InputTopic:
        def __init__(self):
            self.msg  = None
            self.delayed_msg = None
        def __repr__(self):
            return str(dict({"msg": self.msg, "delayed_msg": self.delayed_msg}))

    def __init__(self, filters, config=None):
        super(AdvancedTimeSynchronizer, self).__init__()
        self.lock = threading.Lock()
        self.config = None
        self.inputs = None
        self.never_signaled = True
        self.resample_timer = None
        self.connect_input(filters)
        if config is not None:
            for i,c in enumerate(config):
                if i < len(self.config) and c is not None:
                    self.config[i] = c

    def enable_resampling(self, period):
        self.disable_resampling()
        self.resample_timer = rospy.Timer(period, self.callback_resample)

    def disable_resampling(self):
        if self.resample_timer is not None:
            self.resample_timer.shutdown()

    def callback_resample(self, event):
        with self.lock:
            self._try_send()

    def connect_input(self, filters):
        self.config = [AdvancedTimeSynchronizer.TopicConfig() for f in filters]
        self.inputs = [AdvancedTimeSynchronizer.InputTopic() for f in filters]
        self.input_connections = [
            f.registerCallback(self.add, idx)
            for idx, f in enumerate(filters)
        ]

    def add(self, msg, my_index=None):
        my_config = self.config[my_index]
        with self.lock:
            my_config = self.config[my_index]
            my_input = self.inputs[my_index]

            if my_config.delay and my_input.msg is not None:
                my_input.delayed_msg = my_input.msg
            my_input.msg = msg

            if my_config.trigger or self.never_signaled:
                self._try_send()

    def _try_send(self):
        if self._is_complete():
            msgs = [self._extract_msg(c,i) for c,i in zip(self.config, self.inputs)]
            self.signalMessage(*msgs)
            for c,i in zip(self.config, self.inputs):
                if c.clear:
                    if c.delay:
                        i.delayed_msg = None
                    else:
                        i.msg = None
                        
            self.never_signaled = False

    def _is_complete(self):
        return all([
            c.optional 
            or (m is not None) 
            for c,i in zip(self.config, self.inputs) 
            for m in [self._extract_msg(c,i)]
        ])

    def _extract_msg(self, config, input):
        if config.delay:
            return input.delayed_msg
        else:
            return input.msg


## usage example:

# import rospy
# import message_filters
# rospy.init_node('must not be empty', anonymous=True)
# sub_img = message_filters.Subscriber("~in/Image", sensor_msgs.msg.Image)
# sub_param = message_filters.Subscriber("~in/CameraParameter", sensor_msgs.msg.CameraInfo)
# sync = AdvancedTimeSynchronizer([sub_img, sub_param],[
#     # process new images, don't process them twice
#     AdvancedTimeSynchronizer.TopicConfig(trigger=True, delay=False, optional=False, clear=True),
#     AdvancedTimeSynchronizer.TopicConfig(trigger=True, delay=False, optional=False, clear=False),
# ])
# sync.registerCallback(callback)
# def callback(msg_image, msg_camera_parameter):
#     pass
#     