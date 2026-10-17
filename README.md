# esp32-microros-robot-control
A robotics project that uses an ESP32 and micro-ROS to control a differential-drive robot through ROS 2 velocity commands. The ESP32 receives linear and angular velocity commands from the `cmd_vel` topic over Wi-Fi and converts them into motor-control signals. A command timeout feature stops the motors if new commands are not received.
