#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

MPU6050 mpu;

// Button pins
const int TRIGGER_PIN = 2;
const int LEFT_CLICK_PIN = 3; 
const int RIGHT_CLICK_PIN = 4;
const int LED_PIN = 13;

// MPU6050 variables
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Mouse control variables
int mouseX, mouseY;
bool mouseActive = false;
bool leftClick = false;
bool rightClick = false;

// Calibration values (adjust these based on your sensor)
int offsetX = 0;
int offsetY = 0;

void setup() {
    // Initialize serial communication
    Serial.begin(9600);

    // Initialize I2C
    Wire.begin();

    // Initialize MPU6050
    mpu.initialize();

    // Set pin modes
    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    pinMode(LEFT_CLICK_PIN, INPUT_PULLUP);
    pinMode(RIGHT_CLICK_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    // Test MPU6050 connection
    if (mpu.testConnection()) {
        Serial.println("MPU6050 connection successful");
        digitalWrite(LED_PIN, HIGH);
        delay(1000);
        digitalWrite(LED_PIN, LOW);
    } else {
        Serial.println("MPU6050 connection failed");
        while(1) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
        }
    }

    // Calibrate sensor (hold still for 3 seconds)
    delay(3000);
    calibrateSensor();
}

void loop() {
    // Check trigger button
    mouseActive = !digitalRead(TRIGGER_PIN);

    if (mouseActive) {
        digitalWrite(LED_PIN, HIGH);

        // Read MPU6050 data
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        // Convert gyroscope data to mouse movement
        mouseX = map(gx + offsetX, -17000, 17000, -20, 20);
        mouseY = map(gy + offsetY, -17000, 17000, -20, 20);

        // Apply deadzone to reduce jitter
        if (abs(mouseX) < 3) mouseX = 0;
        if (abs(mouseY) < 3) mouseY = 0;

        // Check click buttons
        leftClick = !digitalRead(LEFT_CLICK_PIN);
        rightClick = !digitalRead(RIGHT_CLICK_PIN);

        // Send data to Python
        Serial.print(mouseX);
        Serial.print(",");
        Serial.print(mouseY);
        Serial.print(",");
        Serial.print(leftClick ? "1" : "0");
        Serial.print(",");
        Serial.println(rightClick ? "1" : "0");

    } else {
        digitalWrite(LED_PIN, LOW);
    }

    delay(20); // Small delay for stability
}

void calibrateSensor() {
    long sumX = 0, sumY = 0;
    int samples = 100;

    for (int i = 0; i < samples; i++) {
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        sumX += gx;
        sumY += gy;
        delay(10);
    }

    offsetX = -(sumX / samples);
    offsetY = -(sumY / samples);

    Serial.print("Calibration complete. OffsetX: ");
    Serial.print(offsetX);
    Serial.print(", OffsetY: ");
    Serial.println(offsetY);
}