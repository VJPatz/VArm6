# VArm6 — 6DOF Open Source Joystick-Controlled Robotic Arm

![Platform](https://img.shields.io/badge/platform-Arduino%20Mega%202560-blue)
![Framework](https://img.shields.io/badge/framework-PlatformIO-orange)
![License](https://img.shields.io/badge/license-MIT-green)

A fully open-source, 6-degree-of-freedom robotic arm controlled by three analog joysticks. Built on an Arduino Mega 2560 + PCA9685 PWM driver. Designed to be hacked, extended, and rebuilt.

[![Watch the build](https://img.youtube.com/vi/pzN0e9XsAOU/maxresdefault.jpg)](https://youtu.be/pzN0e9XsAOU)

**[▶ Watch the full build video](https://youtu.be/pzN0e9XsAOU)**

---

## Features

- 6 independent servo joints — Base, Shoulder, Elbow, Wrist, Rotate, Grip
- Preventing sloppy multi-joint drift
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

## Project Structure

```
VArm6/
├── src/
│   └── main.cpp           ← All control logic
├── schematic/
│   ├── VArm6.kicad_sch    ← KiCad schematic source
│   └── schematic.pdf      ← Printable wiring reference
├── platformio.ini          ← Build config (Mega 2560, Arduino framework)
└── README.md
```

---

## 3D Models Used

- **Printed model:** [Robotic Arm with Servo Arduino — MakerWorld](https://makerworld.com/en/models/1134925-robotic-arm-with-servo-arduino?from=search#profileId-1135927)
- **Laser-cut / assembled model:** [Thing:4637503 — Thingiverse](https://www.thingiverse.com/thing:4637503)

---

## Extending This Project

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
