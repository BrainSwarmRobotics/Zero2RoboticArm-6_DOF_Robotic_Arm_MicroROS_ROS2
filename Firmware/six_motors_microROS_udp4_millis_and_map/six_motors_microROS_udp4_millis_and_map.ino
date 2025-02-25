#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/float64_multi_array.h>

#include <ESP32Servo.h>

rcl_subscription_t subscriber;
std_msgs__msg__Float64MultiArray msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) { error_loop(); }}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) {} }


Servo servo1, servo2, servo3, servo4, servo5, servo6;
int servo1Pin = 12; //Base
int servo2Pin = 13; //Wrist
int servo3Pin = 14; // Elbow
int servo4Pin = 15; //Shoulder 1
int servo5Pin = 21; //Shoulder 2
int servo6Pin = 22; //Gripper

float currentAngle1 = 0, targetAngle1;
float currentAngle2 = 0, targetAngle2; // Reverse direction
float currentAngle3 = 0, targetAngle3; // Reverse direction
float currentAngle4 = 0, targetAngle4; // Reverse direction
float currentAngle5 = 0, targetAngle5; // Reverse direction
float currentAngle6 = 0, targetAngle6; // Reverse direction

const float stepSize = 1.0; // Degrees per update
unsigned long lastUpdateTime = 0;
const int updateInterval = 10; // Milliseconds

void error_loop() {
  while (1) {
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
  }
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// ROS 2 Callback Function
void subscription_callback(const void *msgin) {
  const std_msgs__msg__Float64MultiArray *msg = (const std_msgs__msg__Float64MultiArray *)msgin;

  if (msg->data.size == 6) {

      float radianValue1 = msg->data.data[0];
      float radianValue2 = msg->data.data[1];
      float radianValue3 = msg->data.data[2];
      float radianValue4 = msg->data.data[3];
      float radianValue5 = msg->data.data[4];
      float radianValue6 = msg->data.data[5];

      // Convert radians (-1.57 to 1.57) to microseconds (PWM)
      targetAngle1 = mapFloat(radianValue1, -1.57, 1.57, 0, 180);
      targetAngle2 = mapFloat(radianValue2, -1.57, 1.57, 0, 180);
      targetAngle3 = mapFloat(radianValue3, -1.57, 1.57, 0, 180);
      targetAngle4 = mapFloat(radianValue4, -1.57, 1.57, 0, 180);
      targetAngle5 = mapFloat(radianValue5, -1.57, 1.57, 0, 180);
      targetAngle6 = mapFloat(radianValue6, -1.57, 1.57, 0, 180);

  }
}


void setup() {
    Serial.begin(115200);
    Serial.println("Starting Micro-ROS ESP32 Node");

    set_microros_wifi_transports("<Your SSID>", "<WiFi Password>", "<MicroROS Agent IP>", 8888);

    servo1.setPeriodHertz(50);
    servo2.setPeriodHertz(50);
    servo3.setPeriodHertz(50);
    servo4.setPeriodHertz(50);
    servo5.setPeriodHertz(50);
    servo6.setPeriodHertz(50);

    servo1.attach(servo1Pin, 500, 2400);
    servo2.attach(servo2Pin, 500, 2400);
    servo3.attach(servo3Pin, 500, 2400);
    servo4.attach(servo4Pin, 1000, 2000);
    servo5.attach(servo5Pin, 1000, 2000);
    servo6.attach(servo6Pin, 1000, 2000);

    // Set initial positions
    servo1.writeMicroseconds(mapFloat(currentAngle1, 0, 180, 500, 2400));
    servo2.writeMicroseconds(mapFloat(currentAngle2, 0, 180, 500, 2400));
    servo3.writeMicroseconds(mapFloat(currentAngle3, 0, 180, 500, 2400));
    servo4.writeMicroseconds(mapFloat(currentAngle4, 0, 180, 1000, 2000));
    servo5.writeMicroseconds(mapFloat(currentAngle5, 0, 180, 1000, 2000));
    servo6.writeMicroseconds(mapFloat(currentAngle6, 0, 180, 1000, 2000));



  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  RCCHECK(rclc_node_init_default(&node, "hardware_driver_node", "", &support));

  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  qos_profile.depth = 1;
  qos_profile.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;

  RCCHECK(rclc_subscription_init(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float64MultiArray),
    "motor_command",
    &qos_profile));

  std_msgs__msg__Float64MultiArray__init(&msg);
  size_t array_size = 6;
  msg.data.data = (double *)malloc(array_size * sizeof(double));
  msg.data.size = array_size;
  msg.data.capacity = array_size;

  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));

}

void loop() {
    unsigned long now = millis();

    if (now - lastUpdateTime >= updateInterval) {
        lastUpdateTime = now;

        // Move servo1 smoothly
        if (currentAngle1 < targetAngle1) {
            currentAngle1 += stepSize;
            if (currentAngle1 > targetAngle1) currentAngle1 = targetAngle1;
        } else if (currentAngle1 > targetAngle1) {
            currentAngle1 -= stepSize;
            if (currentAngle1 < targetAngle1) currentAngle1 = targetAngle1;
        }

        // Move servo2 smoothly (opposite direction)
        if (currentAngle2 < targetAngle2) {
            currentAngle2 += stepSize;
            if (currentAngle2 > targetAngle2) currentAngle2 = targetAngle2;
        } else if (currentAngle2 > targetAngle2) {
            currentAngle2 -= stepSize;
            if (currentAngle2 < targetAngle2) currentAngle2 = targetAngle2;
        }

        if (currentAngle3 < targetAngle3) {
            currentAngle3 += stepSize;
            if (currentAngle3 > targetAngle3) currentAngle3 = targetAngle3;
        } else if (currentAngle3 > targetAngle3) {
            currentAngle3 -= stepSize;
            if (currentAngle3 < targetAngle3) currentAngle3 = targetAngle3;
        }

        if (currentAngle4 < targetAngle4) {
            currentAngle4 += stepSize;
            if (currentAngle4 > targetAngle4) currentAngle4 = targetAngle4;
        } else if (currentAngle4 > targetAngle4) {
            currentAngle4 -= stepSize;
            if (currentAngle4 < targetAngle4) currentAngle4 = targetAngle4;
        }

        if (currentAngle5 < targetAngle5) {
            currentAngle5 += stepSize;
            if (currentAngle5 > targetAngle5) currentAngle5 = targetAngle5;
        } else if (currentAngle5 > targetAngle5) {
            currentAngle5 -= stepSize;
            if (currentAngle5 < targetAngle5) currentAngle5 = targetAngle5;
        }

        if (currentAngle6 < targetAngle6) {
            currentAngle6 += stepSize;
            if (currentAngle6 > targetAngle6) currentAngle6 = targetAngle6;
        } else if (currentAngle6 > targetAngle6) {
            currentAngle6 -= stepSize;
            if (currentAngle6 < targetAngle6) currentAngle6 = targetAngle6;
        }

        // Convert angle to pulse width (500µs - 2400µs)
        int pulseWidth1 = mapFloat(currentAngle1, 0, 180, 500, 2400);
        int pulseWidth2 = mapFloat(currentAngle2, 0, 180, 500, 2400);
        int pulseWidth3 = mapFloat(currentAngle3, 0, 180, 500, 2400);
        int pulseWidth4 = mapFloat(currentAngle4, 0, 120, 1000, 2000);
        int pulseWidth5 = mapFloat(currentAngle5, 0, 120, 1000, 2000);
        int pulseWidth6 = mapFloat(currentAngle6, 0, 120, 1000, 2000);

        servo1.writeMicroseconds(pulseWidth1);
        servo2.writeMicroseconds(pulseWidth2);
        servo3.writeMicroseconds(pulseWidth3);
        servo4.writeMicroseconds(pulseWidth4);
        servo5.writeMicroseconds(pulseWidth5);
        servo6.writeMicroseconds(pulseWidth6);




    }
    RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
