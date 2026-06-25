# VArm6 - 6DOF Open Source Joystick-Controlled Robotic Arm

![Platform](https://img.shields.io/badge/platform-Arduino%20Mega%202560-blue)
![Framework](https://img.shields.io/badge/framework-PlatformIO-orange)
![License](https://img.shields.io/badge/license-MIT-green)

A fully open-source codestack, 6-degree-of-freedom robot arm controlled by three analog joysticks. Built on Arduino Mega 2560 + PCA9685 PWM driver. Designed to be hacked, extended, and rebuilt.

---

## What it does

- Controls 6 independent servo joints: Base, Shoulder, Elbow, Wrist, Rotate, Grip
- Diagonal input blocking — only one axis per joystick moves at a time, preventing sloppy multi-joint drift
- Momentum-based motion with configurable acceleration and deceleration
- One-button hold to glide all joints back to a safe home position
- PCA9685 I2C health check on boot — halts with error if driver is missing

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | Arduino Mega 2560 |
| PWM Driver | PCA9685 (I2C, address `0x40`) |
| Servos | 6× hobby servos (SG90 / MG996R or equivalent) |
| Joysticks | 3× dual-axis analog joystick modules |
| Power | Servos need a **separate 5–6V supply** — do not power from Arduino 5V |

---

## Wiring

### Joysticks → Arduino Mega

| Joystick | Axis | Pin | Controls |
|---|---|---|---|
| J1 | X | A8 | Base |
| J1 | Y | A9 | Shoulder |
| J2 | X | A10 | Elbow |
| J2 | Y | A11 | Wrist |
| J3 | X | A12 | Rotate |
| J3 | Y | A13 | Grip |
| J1 | Button | D18 | Home (hold) |

### PCA9685 → Arduino Mega

| PCA9685 Pin | Mega Pin |
|---|---|
| SDA | Pin 20 |
| SCL | Pin 21 |
| VCC | 3.3V or 5V |
| GND | GND |
| V+ | External servo power (5–6V) |

### PCA9685 → Servos

| Channel | Joint |
|---|---|
| 0 | Base |
| 1 | Shoulder |
| 2 | Elbow |
| 3 | Wrist |
| 4 | Rotate |
| 5 | Grip |

> ⚠️ **Always use a separate power supply for servos.** Drawing servo current through the Arduino will cause resets or damage.

---

## Circuit Schematic

Schematic designed in KiCad — see [`/schematic`](./schematic/) for:
- `VArm6.kicad_sch` — editable source file
- `schematic.pdf` — printable reference

---

## Getting Started

### 1. Clone the repo

```bash
git clone https://github.com/VJPatz/VArm6.git
cd VArm6
```

### 2. Install PlatformIO

If you haven't already:

```bash
pip install platformio
```

Or use the [PlatformIO IDE extension for VS Code](https://platformio.org/install/ide?install=vscode).

### 3. Build and flash

```bash
pio run --target upload
```

Make sure your Mega is connected via USB and the correct port is detected. PlatformIO handles library installs automatically.

### 4. Open Serial Monitor (optional)

```bash
pio device monitor --baud 9600
```

You'll see boot confirmation and home status messages.

---

## Configuration

All tunable parameters are clearly grouped at the top of `src/main.cpp`:

| Parameter | Default | What it does |
|---|---|---|
| `homeAngle[6]` | `{90,160,120,90,90,70}` | Safe resting pose for each joint |
| `minAngle[6]` | See file | Minimum travel limit per joint |
| `maxAngle[6]` | See file | Maximum travel limit per joint |
| `SERVOMIN` / `SERVOMAX` | `120` / `600` | PWM pulse range — tune for your specific servos |
| `LOW_TH` / `HIGH_TH` | `300` / `750` | Joystick dead-zone thresholds |
| `UPDATE_INTERVAL` | `25` ms | Motion refresh rate — increase to slow things down |
| `MAX_SPEED` | `2` | Degrees per tick, max |
| `ACCEL` / `DECEL` | `1` | Ramp-up / coast-down rate |

---

## Project Structure

```
VArm6/
├── src/
│   └── main.cpp          ← All control logic
├── schematic/
│   ├── varm6.kicad_sch ← KiCad schematic source
│   └── schematic.pdf     ← Printable wiring reference
├── platformio.ini         ← Build config (Mega 2560, Arduino framework)
└── README.md
```

---

## Extending This Project

A few directions the community can take this:

- **Serial / Bluetooth control** — replace joystick reads with UART commands from a phone or PC
- **Position recording & playback** — log joint positions over time, replay on demand
- **Inverse kinematics** — calculate joint angles from a target XYZ coordinate instead of direct joint control
- **Web UI** — pair with an ESP32 and serve a browser-based controller
- **ROS integration** — publish joint states to a ROS topic for simulation and planning

---

## Dependencies

Managed automatically by PlatformIO via `platformio.ini`:

- [Adafruit PWM Servo Driver Library](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)
- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO)

---

## License

MIT — use it, modify it, build something better.

---

## Contributing

PRs welcome. If you build a version of this arm, open an issue and share it — would love to see what people make.
