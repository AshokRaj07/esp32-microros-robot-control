# ESP32 micro-ROS Robot Control

## About

This project demonstrates how to control a differential-drive robot using an ESP32, micro-ROS, and ROS 2.

The ESP32 connects to a Wi-Fi network and subscribes to the `cmd_vel` topic. It receives linear and angular velocity commands and converts them into motor-control signals to move the robot.

## Features

* Wi-Fi communication using ESP32
* micro-ROS integration with ROS 2
* Subscription to the `cmd_vel` velocity-command topic
* Differential-drive motor control
* PWM-based motor speed control
* Automatic motor stopping when commands time out

## Technologies Used

* ESP32
* Arduino IDE
* micro-ROS
* ROS 2
* C++
* Wi-Fi
* DC motors and motor driver

## Hardware Requirements

* ESP32 development board
* Two DC motors
* Compatible motor driver
* Robot chassis and wheels
* Suitable motor power supply
* Jumper wires
* Wi-Fi network

## Software Requirements

* Arduino IDE
* ESP32 board support package
* Compatible micro-ROS Arduino library
* ROS 2 installation
* A compatible micro-ROS Agent
* `geometry_msgs` support

## How It Works

1. The ESP32 connects to the configured Wi-Fi network.
2. It initializes the micro-ROS node named `esp32_drive`.
3. The node subscribes to the `cmd_vel` topic.
4. Incoming linear and angular velocity commands are converted into left- and right-motor commands.
5. PWM signals control the motor direction and speed.
6. If no new command arrives within the configured timeout, the motors stop.

## ROS 2 Topic

| Property      | Value                     |
| ------------- | ------------------------- |
| Topic         | `/cmd_vel`                |
| Message type  | `geometry_msgs/msg/Twist` |
| Node name     | `esp32_drive`             |
| Communication | Wi-Fi using micro-ROS     |

## Safety Feature

The program includes a command timeout. If no new velocity command is received for 500 milliseconds, the motor outputs are set to zero.

This is a basic communication-loss safeguard, not a replacement for a physical emergency stop or other robot safety measures.

## Configuration

Before uploading the program, configure the following values in the Arduino sketch:

* Wi-Fi network name
* Wi-Fi password
* micro-ROS Agent IP address
* micro-ROS Agent port
* Motor driver pin connections

Use your own network details. Do not publish real Wi-Fi credentials or other private connection information.

## Example ROS 2 Commands

To inspect the topic:

```bash
ros2 topic list
```

To check the incoming velocity commands:

```bash
ros2 topic echo /cmd_vel
```

To publish a forward movement command:

```bash
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.2}, angular: {z: 0.0}}"
```

To publish a rotation command:

```bash
ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.0}, angular: {z: 0.5}}"
```

**Important:** These commands require a working ROS 2 and micro-ROS Agent setup, correct network configuration, and compatible hardware. Test motor commands with the robot safely supported before operating it on the ground.

## Future Improvements

* Integrate keyboard-based teleoperation
* Add wheel encoders for feedback
* Implement closed-loop speed control
* Add battery-voltage monitoring
* Improve connection-status reporting

## Learning Outcomes

This project explores micro-ROS communication, ROS 2 topics, ESP32 Wi-Fi connectivity, PWM motor control, differential-drive kinematics, and basic command-timeout handling.
