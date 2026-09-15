# Smart_Landslide_Detection_and_Early_Warning_System_Using_PIC_and_MPU9250
Real-time embedded landslide risk monitoring using soil moisture and MPU9250 sensors.

## About the Project

Landslides can occur when the soil becomes highly saturated and the ground starts to shift. Detecting these changes early can help provide a warning before the situation becomes dangerous.

This project is a **Smart Landslide Detection and Early Warning System** developed using a **PIC microcontroller, MPU9250 accelerometer, and soil moisture sensor**.

The system continuously monitors the moisture level of the soil and the movement/tilt of the ground. Based on predefined threshold values, it identifies whether the area is safe, at risk due to high moisture, experiencing ground movement, or showing conditions that indicate a possible landslide.

The current status is displayed on an **I²C LCD**, while a **buzzer** provides an audible warning. Sensor values and system status can also be monitored through **UART/Serial Monitor**.

---

## Main Features

* Real-time soil moisture monitoring
* Ground movement detection using MPU9250
* LCD display for system status
* Buzzer-based warning system
* UART serial monitoring
* Dual-parameter landslide detection
* Simple threshold-based risk classification
* Low-cost embedded system implementation

---

## Components Used

### Hardware

* PIC Microcontroller
* MPU9250 IMU / Accelerometer
* Soil Moisture Sensor
* 16×2 I²C LCD
* Buzzer
* 20 MHz Crystal
* Power Supply
* Connecting wires / Breadboard

### Software

* Embedded C
* MPLAB X IDE
* XC8 Compiler

---

## How the System Works

The system mainly uses two parameters to identify landslide risk:

### 1. Soil Moisture

The soil moisture sensor is connected to the ADC input of the PIC microcontroller.

The ADC reading is converted into a percentage value.

The current program considers the soil to have a high moisture risk when:

```text
Moisture > 75%
```

### 2. Ground Movement

The MPU9250 is used to measure acceleration along the X, Y and Z axes.

For the current detection algorithm, the X and Y axis readings are checked against a threshold of:

```text
±0.40 g
```

If either axis crosses this limit, the system considers that significant ground movement/tilt has been detected.

---

## Risk Detection Logic

The system combines soil moisture and ground movement to determine the current condition.

| Soil Moisture | Ground Movement | Status                |
| ------------- | --------------- | --------------------- |
| ≤ 75%         | Not detected    | SAFE                  |
| > 75%         | Not detected    | HIGH RISK – WET SOIL  |
| ≤ 75%         | Detected        | GROUND SHIFT DETECTED |
| > 75%         | Detected        | LANDSLIDE ALERT       |

When both high soil moisture and significant ground movement are detected, the buzzer remains ON and the LCD displays a landslide warning.

---

## System Block Diagram

```text
             ┌───────────────────┐
             │ Soil Moisture     │
             │     Sensor        │
             └─────────┬─────────┘
                       │ ADC
                       │
                       ▼
                ┌───────────────┐
                │ PIC           │
                │ Microcontroller│
                └───────┬───────┘
                        ▲
                        │ I²C
                        │
                ┌───────┴────────┐
                │    MPU9250     │
                │  Accelerometer │
                └────────────────┘
                        │
             ┌──────────┼──────────┐
             │          │          │
             ▼          ▼          ▼
          I²C LCD     Buzzer     UART
          Display     Warning    Monitor
```

---

## Output

The LCD displays the current soil moisture and risk condition.

For example:

```text
M:085%  DANGER!!
LANDSLIDE ALERT
```

The UART output provides more detailed information:

```text
--- LANDSLIDE MONITORING ---
Moisture : 85%
Tilt X   : 0.52g
Tilt Y   : 0.18g
>> ALERT: LANDSLIDE DETECTED! <<
```

---

## Pin / Interface Overview

| Module               | PIC Interface |
| -------------------- | ------------- |
| Soil Moisture Sensor | ADC / AN0     |
| MPU9250              | I²C           |
| I²C LCD              | I²C           |
| Buzzer               | RB0           |
| UART TX              | RC6           |
| UART RX              | RC7           |

---

## Project Workflow

```text
Start
  ↓
Initialize PIC Peripherals
  ↓
Initialize LCD
  ↓
Initialize MPU9250
  ↓
Read Soil Moisture
  ↓
Read MPU9250
  ↓
Calculate Moisture & Acceleration
  ↓
Check Thresholds
  ↓
Determine Risk Level
  ↓
Display Status on LCD
  ↓
Activate Buzzer if Required
  ↓
Send Data through UART
  ↓
Repeat
```

---

## Project Status

The prototype successfully demonstrates real-time monitoring of soil moisture and ground movement and provides local warnings through the LCD and buzzer.

The system is mainly intended as an **embedded systems and early-warning prototype**. The threshold values used in the current implementation are fixed and would need proper calibration for use in a real geographical location.

---

## Future Improvements

Some improvements that can be added in future versions include:

* Adding GSM/LoRa/Wi-Fi for remote alerts
* Sending notifications to a mobile application
* Adding rainfall measurement
* Using multiple soil and motion sensors
* Storing sensor readings for later analysis
* Applying filtering to reduce sensor noise
* Using location-specific threshold values
* Adding GPS for identifying the affected location
* Developing a cloud-based monitoring dashboard

---

## Project Purpose

The main aim of this project is to demonstrate how simple embedded sensors can be combined to monitor environmental conditions and provide an early warning when potentially dangerous changes are detected.

It also provides practical experience with **ADC, I²C, UART, GPIO, sensor interfacing, LCD interfacing, and embedded C programming** using a PIC microcontroller.

---

## Disclaimer

This project is an educational prototype and should not be used as a certified system for predicting or preventing real-world landslides. Actual landslide monitoring requires geological studies, proper sensor calibration, multiple measurement points, and professional safety systems.

---

## Author

Developed as an academic microcontroller project.

**Technologies:** PIC Microcontroller • Embedded C • MPU9250 • Soil Moisture Sensor • I²C • UART • ADC
