#include <micro_ros_arduino.h>
#include <WiFi.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <geometry_msgs/msg/twist.h>

// -------- Wi-Fi Config ----------
char ssid_c[]     = "BOT";
char password_c[] = "12345678";

// -------- Agent Info ----------
char agent_ip_c[] = "10.10.5.111";
uint32_t agent_port = 8888;

// -------- Motor Pins (4-pin driver, no ENA/ENB) ----------
const int M1A = 25;   // Left  motor forward
const int M1B = 26;   // Left  motor reverse
const int M2A = 27;   // Right motor forward
const int M2B = 14;   // Right motor reverse

// -------- Tuning ----------
const float MAX_LINEAR  = 0.5;   // m/s that equals full speed (teleop default 0.5)
const float MAX_ANGULAR = 1.0;   // rad/s that equals full turn (teleop default 1.0)
const int   MIN_PWM     = 60;    // minimum PWM for motors to actually start moving
const unsigned long CMD_TIMEOUT_MS = 500;  // stop if no command received

// -------- PWM helper (works on ESP32 core 2.x and 3.x) ----------
const int PWM_FREQ = 1000;
const int PWM_RES  = 8;  // 0-255

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  void pwmInit(int pin, int ch) { ledcAttach(pin, PWM_FREQ, PWM_RES); }
  void pwmWrite(int pin, int ch, int val) { ledcWrite(pin, val); }
#else
  void pwmInit(int pin, int ch) { ledcSetup(ch, PWM_FREQ, PWM_RES); ledcAttachPin(pin, ch); }
  void pwmWrite(int pin, int ch, int val) { ledcWrite(ch, val); }
#endif

// -------- micro-ROS objects ----------
rcl_subscription_t subscriber;
rclc_executor_t executor;
rcl_node_t node;
rcl_allocator_t allocator;
rclc_support_t support;
geometry_msgs__msg__Twist msg;

unsigned long lastCmdTime = 0;

// Drive one motor: speed -255..255
void driveMotor(int pinA, int chA, int pinB, int chB, int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0) {
    pwmWrite(pinA, chA, speed);
    pwmWrite(pinB, chB, 0);
  } else if (speed < 0) {
    pwmWrite(pinA, chA, 0);
    pwmWrite(pinB, chB, -speed);
  } else {
    pwmWrite(pinA, chA, 0);
    pwmWrite(pinB, chB, 0);
  }
}

int toPwm(float v) {            // v: -1.0..1.0 -> -255..255 with deadzone
  if (fabs(v) < 0.05) return 0;
  int pwm = (int)(fabs(v) * (255 - MIN_PWM)) + MIN_PWM;
  return v > 0 ? pwm : -pwm;
}

void stopMotors() {
  driveMotor(M1A, 0, M1B, 1, 0);
  driveMotor(M2A, 2, M2B, 3, 0);
}

// -------- Callback ----------
void cmd_vel_callback(const void * msgin) {
  const geometry_msgs__msg__Twist * m = (const geometry_msgs__msg__Twist *)msgin;

  float lin = constrain(m->linear.x  / MAX_LINEAR,  -1.0, 1.0);
  float ang = constrain(m->angular.z / MAX_ANGULAR, -1.0, 1.0);

  // Differential drive mixing
  float left  = lin - ang;
  float right = lin + ang;

  // Normalize if either exceeds 1.0
  float maxMag = max(fabs(left), fabs(right));
  if (maxMag > 1.0) { left /= maxMag; right /= maxMag; }

  int leftPwm  = toPwm(left);
  int rightPwm = toPwm(right);

  driveMotor(M1A, 0, M1B, 1, leftPwm);
  driveMotor(M2A, 2, M2B, 3, rightPwm);

  lastCmdTime = millis();

  Serial.printf("lin=%.2f ang=%.2f | L=%d R=%d\n",
                m->linear.x, m->angular.z, leftPwm, rightPwm);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ESP32 micro-ROS drive...");

  pwmInit(M1A, 0);
  pwmInit(M1B, 1);
  pwmInit(M2A, 2);
  pwmInit(M2B, 3);
  stopMotors();

  // Wi-Fi + micro-ROS transport (this connects Wi-Fi itself)
  set_microros_wifi_transports(ssid_c, password_c, agent_ip_c, agent_port);
  delay(2000);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "esp32_drive", "", &support);

  rclc_subscription_init_default(
    &subscriber, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "cmd_vel");

  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_subscription(&executor, &subscriber, &msg, &cmd_vel_callback, ON_NEW_DATA);

  lastCmdTime = millis();
  Serial.println("✅ micro-ROS drive ready!");
}

void loop() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));

  // Failsafe: stop if no cmd_vel for a while
  if (millis() - lastCmdTime > CMD_TIMEOUT_MS) {
    stopMotors();
  }
}
