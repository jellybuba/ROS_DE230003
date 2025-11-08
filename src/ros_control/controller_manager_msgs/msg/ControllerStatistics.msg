# This message contains the state of one realtime controller# that was spawned in the controller manager# the name of the controllerstring name
# the type of the controllerstring type
# the time at which these controller statistics were measuredbuiltin_interfaces/Time timestamp
# bool that indicates if the controller is currently# in a running or a stopped statebool running
# the maximum time the update loop of the controller ever needed to completebuiltin_interfaces/Duration max_time
# the average time the update loop of the controller needs to complete.# the average is computed in a sliding time window.builtin_interfaces/Duration mean_time
# the variance on the time the update loop of the controller needs to complete.# the variance applies to a sliding time window.builtin_interfaces/Duration variance_time
# the number of times this controller broke the realtime loopint32 num_control_loop_overruns
# the timestamp of the last time this controller broke the realtime loopbuiltin_interfaces/Time time_last_control_loop_overrun
