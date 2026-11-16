# Blixathon Line Follower

An Arduino competition robot that follows a contrasting track and attempts to recover when it loses the line.

## First run

Start with the hardware and wiring notes below. Open [line_follower.ino](line_follower.ino) in the Arduino IDE, select the board matching your hardware, and upload. Check readings over the track and background before tuning the control parameters. Test at low speed with clearance around the robot.

**Team Name:** Valley Boys Robotics
**Project:** Autonomous Line Follower Robot (QTR-8RC)
**Platform:** Arduino Uno + L298N + QTR-8RC (8-sensor) Array
**Category:** Hardware-Programming (Coding Quest 2025)

---

## 🧭 Overview

This project demonstrates a fully autonomous **line-following robot** capable of detecting and following a black path on a white surface using a QTR-8RC infrared sensor array and a custom PD-controlled steering algorithm.

The robot dynamically adjusts its motor speeds to maintain precise alignment with the track while automatically recovering from line-loss scenarios.

It was designed according to the **Coding Quest competition guidelines**, emphasizing:

* Efficient hardware–software integration,
* Autonomous control logic (no manual input),
* Smooth navigation, and
* Intelligent calibration and recovery.

---

## 🧠 Core Features

| Feature                 | Description                                                                                                |
| ----------------------- | ---------------------------------------------------------------------------------------------------------- |
| **Self-Calibration**    | Automatically detects black/white reflectance thresholds using built-in QTR calibration.                   |
| **PD Control**          | Combines proportional and derivative control to minimize oscillations and improve line-tracking stability. |
| **Centroid Weighting**  | Computes the line position using all 8 sensors for smooth steering.                                        |
| **Smart Recovery**      | Detects when the line is lost and performs staged recovery (coast → arc → spin).                           |
| **Real-Time Tuning**    | Adjust Kp, Kd, speed, and boost through the Serial Monitor without reuploading code.                       |
| **Optimized for L298N** | Deadband compensation and PWM speed shaping for low-voltage operation.                                     |

---

## ⚙️ Hardware Configuration

| Component              | Function                 | Notes                                           |
| ---------------------- | ------------------------ | ----------------------------------------------- |
| **Arduino Uno**        | Main controller          | Handles QTR sensor timing + PD computation      |
| **Pololu QTR-8RC**     | Reflectance sensor array | 8 RC sensors (digital timing-based reflectance) |
| **L298N Motor Driver** | Motor control            | Dual H-bridge controlling both DC motors        |
| **DC Gear Motors**     | Drive motors             | 6V-12V rated, 100-300 RPM recommended           |
| **Power Supply**       | 7.4–12V Li-ion / Li-Po   | Directly powers L298N; 5V line powers Arduino   |
| **Chassis**            | Acrylic / custom-cut MDF | Balanced weight and 2–3 mm sensor clearance     |

---

## 🔌 Wiring Summary

| Module  | Pin       | Arduino Pin                            | Purpose                  |
| ------- | --------- | -------------------------------------- | ------------------------ |
| QTR-8RC | OUT1–OUT8 | D2, D3, D4, D5, D7, A2(16), D8, A0(14) | Sensor inputs            |
| QTR-8RC | LEDON     | A1 (15)                                | Optional emitter control |
| L298N   | ENA       | D10                                    | Right motor PWM          |
| L298N   | IN1       | D11                                    | Right motor IN1          |
| L298N   | IN2       | D12                                    | Right motor IN2          |
| L298N   | IN3       | D13                                    | Left motor IN3           |
| L298N   | IN4       | D9                                     | Left motor IN4           |
| L298N   | ENB       | D6                                     | Left motor PWM           |
| Power   | 5V / GND  | Shared                                 | All modules              |

**Sensor height:** ~2–3 mm above track
**Optimal lighting:** Diffuse indoor LED lighting, avoid glare

---

## 🧩 Software Architecture

### 1. **Initialization**

* Initializes QTR sensors, performs ~0.6 s self-calibration.
* Sets up motor pins and serial interface.

### 2. **Main Loop**

Runs continuously with three primary stages:

1. **Read line position** using `readLineBlack()` (0–7000 scale).
2. **Compute PD output:**
   [
   \text{steer} = K_p \cdot \text{error} + K_d \cdot (\text{error} - \text{lastError})
   ]
3. **Adjust motor PWM:**

   * Turns proportionally to error magnitude
   * Adds a small boost when running straight
   * Clamps steering and ensures minimum PWM

### 3. **Recovery Logic**

If all sensors detect white:

* 0–120 ms → coast straight
* 120–450 ms → arc in last known direction
* > 450 ms → in-place spin until line reacquired

---

## 📈 Control Parameters

| Parameter    | Description         | Typical Range |
| ------------ | ------------------- | ------------- |
| `Kp`         | Proportional gain   | 0.18–0.30     |
| `Kd`         | Derivative gain     | 0.9–1.3       |
| `BASE_SPEED` | Cruise PWM          | 80–110        |
| `MIN_PWM`    | Minimum motor PWM   | 55–70         |
| `BOOST`      | Straight-line boost | 10–25         |

> Tune parameters through the Serial Monitor:
>
> * `+` / `-` → adjust Kp
> * `>` / `<` → adjust Kd
> * `]` / `[` → adjust base speed
> * `)` / `(` → adjust boost
> * `S` → print current settings

---

## 🧪 Calibration Procedure

1. Place the robot so that all sensors see both black line and white surface.
2. Upload and start the code — it auto-calibrates in the first 0.6 s.
3. If needed, manually trigger recalibration by resetting the board.
4. Use Serial Monitor to verify stable calibration (WHITE ≈ low values, BLACK ≈ high).

---

## 🏁 Competition Alignment

| Competition Spec                     | Implementation                                  |
| ------------------------------------ | ----------------------------------------------- |
| **Autonomous navigation only**       | Fully self-driven, no remote input              |
| **Defined line path**                | 8-sensor centroid ensures robust detection      |
| **Recovery from gaps/intersections** | Multi-stage recovery algorithm                  |
| **Energy efficiency**                | PWM-based dynamic speed control                 |
| **Presentation requirement**         | Code + schematic + documentation provided       |
| **Innovation criteria**              | PD tuning, dynamic boost, serial runtime tuning |

---

## 📦 File Structure

```
/LFR_QTR8RC_PD/
│
├── LFR_QTR8RC_PD.ino       # Main Arduino sketch
├── README.md               # This documentation
├── wiring_diagram.png      # Optional Fritzing diagram
└── LICENSE                 # Open-source license (optional)
```

---

## 🧑‍💻 Team & Contributions

| Member                          | Role                  | Contribution                                    |
| ------------------------------- | --------------------- | ----------------------------------------------- |
| **Anirudh Kishore (Invictine)** | Team Lead, Programmer | Control algorithm, PD tuning, code optimization |
| **Hardware Team**               | Build & wiring        | Sensor array integration, chassis balancing     |
| **Design Team**                 | CAD / Enclosure       | Weight distribution, sensor positioning         |
| **Documentation Lead**          | Technical report      | README, presentation & competition compliance   |

---

## 🛠️ Future Improvements

* Add **integral term (PID)** with adaptive tuning for varying track brightness.
* Implement **EEPROM save/load** for calibration & tuning data.
* Integrate **color recognition** for future cube-sorting challenges.
* Add **OLED display** for live telemetry (error, PWM, etc.).
* Support **line intersection logic** for multi-path courses.

---

## 📚 License

This project is open-source under the **MIT License**.
You’re free to use, modify, and share the code — attribution appreciated.

---
