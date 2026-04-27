# robo-8000  
esp32 / bluepad32 / arduino project

---

## Arduino IDE setup

- add urls to additional board manager  
  - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json  
  - https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json  
- install library **ArduinoJson by Benoit Blanchon**  
- install board manager **esp32_bluepad32 by Ricardo Quesada**

---

## Windows COM PORT driver

- install CP210x driver  
  https://www.silabs.com/documents/public/software/CP210x_Universal_Windows_Driver.zip  

---

## System overview

```
Gamepad → Input → Mapping → Intent → Control → Motor / GPIO outputs
```

- **CONFIG mode** → WiFi config server active, no driving  
- **RUN mode** → Bluetooth controller + motors active  

---

## Mode switch + indicator LED

| Pin | Function |
|-----|--------|
| GPIO 33 | Mode switch (INPUT_PULLUP) |
| GPIO 2  | Mode LED |

- **Switch HIGH → RUN mode**
- **Switch LOW → CONFIG mode**
- LED **ON = RUN**, **OFF = CONFIG**

---

## Motor

- Type: DC micro gearmotor (N20)
- Voltage: 6V
- Speed: ~500 RPM

---

## Motor driver

- Current: **DRV8833**
- Previous: ~~MX1616H~~

### Motor pinout (ESP32 → driver)

| Motor | IN1 | IN2 |
|------|-----|-----|
| LEFT | GPIO 18 | GPIO 19 |
| RIGHT | GPIO 21 | GPIO 22 |

### Behaviour

| IN1 | IN2 | Result |
|-----|-----|--------|
| PWM | LOW | Forward |
| LOW | PWM | Reverse |
| LOW | LOW | Coast |
| HIGH | HIGH | Brake |

- PWM: 20kHz, 8-bit (0–255)

---

## Motor control API

```cpp
setMotor(LEFT, 100);     // forward
setMotor(RIGHT, -100);   // reverse

setTank(100, 100);       // both forward
setArcade(linear, angular); // main drive method

stopMotors();            // coast
brakeMotors();           // hard stop
```

- Speed range: **-255 → +255**

---

## GPIO outputs (mapping-driven)

From mapping config:

| Function | Pin | Behaviour |
|----------|-----|----------|
| pulse | GPIO 25 | momentary HIGH |
| toggle | GPIO 4 | toggles on press |
| hold | GPIO 5 | follows button |

- Pulse duration configurable (default 120ms)

---

## Controller mapping (default)

- Left stick Y → forward/back
- Left stick X → turning
- RB → boost
- B → stop
- Y → pulse
- X → toggle
- A → hold

---

## Serial commands

| Command | Description |
|--------|------------|
| `mode run` | Switch to RUN mode (motors + controller active) |
| `mode config` | Switch to CONFIG mode (WiFi config server) |
| `mode toggle` | Toggle between RUN and CONFIG |
| `mode` | Print current mode |
| `heartbeat on/off` | Enable/disable loop heartbeat logging |
| `inputlog on/off` | Enable periodic input state logging |
| `buttonlog on/off` | Log button press changes |
| `intentlog on/off` | Log processed movement intent |
| `batterylog on/off` | Include battery % in logs |

---

## Loop timing

- Main loop runs at **50Hz (20ms)**

---

## Safety behaviour

- If controller disconnects → motors stop
- If **stop button (B)** pressed → motors stop
- Deadzone + smoothing applied to sticks

---

## Power notes

- ESP32 powered separately (recommended)
- Motor driver powered from battery (e.g. 2S LiPo)
- **Common ground required**

---

## Known quirks

- MX1616H may not like PWM on both pins (DRV8833 preferred)
- GPIO 2 is shared with LED + pulse output

---

## Next planned improvements

- acceleration limiting
- current limiting / brownout protection
- per-motor calibration
- telemetry streaming