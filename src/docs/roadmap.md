# 🗺️ Roadmap (Clean, Ordered, Buildable — Refined)

## ⚙️ Phase 0 – Core System Skeleton (NEW)

**Goal:** Establish a solid foundation before features

### Tasks:
- Create repo
- create boilerplate folders and files for project
- Define core structures:

```cpp
struct Intent {
    float linear;    // -1.0 → 1.0
    float angular;   // -1.0 → 1.0
    bool boost;
    bool stop;
};

enum Mode {
    CONFIG,
    RUN
};

enum ControllerState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};
```

- Implement fixed loop timing (non-blocking):

```cpp
const int LOOP_INTERVAL_MS = 20; // 50Hz
```

- Create basic update loop:

```cpp
void update() {
    readInputs();
    mapToIntent();
    applySafety();
    outputControl(); // stub for now
}
```

- Add lightweight logging layer:

```cpp
log(INFO, "System starting");
```

### Success Criteria:
- Repo on github exists
- Stable loop timing
- Clean project structure
- Logging works

✅ Phase 0 Completion Checklist (real version)
You’re done when check:
 Code compiles cleanly
 Upload works
 Serial logging works
 Loop runs at stable interval
 No blocking code anywhere
 Project structure feels clean
---

## ✅ Phase 1 – Bluetooth Controller (Input Layer)

**Goal:** Receive and understand controller input

### Tasks:
- Connect controller via Bluetooth (PS4Controller or BLE HID)
- Implement controller state machine
- Print all inputs to Serial
- Normalize values (-1.0 → 1.0)
- Add deadzone handling
- Add input smoothing:
```cpp
smoothed = alpha * new + (1 - alpha) * old;
```

### Success Criteria:
- Reliable connection
- No jitter
- Clean state transitions

---

## ✅ Phase 2 – Mapping System (Abstraction Layer)

**Goal:** Separate input from behavior

### Tasks:
- Define mapping JSON (config only)
- Parse JSON once at startup
- Convert to internal struct
- Build translation layer:
```
controller input → mapping → Intent
```

### Success Criteria:
- Mapping change requires no code change
- Outputs structured Intent

---

## ✅ Phase 3 – Preferences Storage (Persistence Layer)

**Goal:** Persist configuration safely

### Tasks:
- Use ESP32 Preferences (NVS)
- Store WiFi, mapping JSON, API endpoint
- Implement load/save
- Add validation + fallback defaults

### Success Criteria:
- Config survives reboot
- Invalid config safe

---

## ✅ Phase 4 – Web Server (Config Mode UI)

**Goal:** Configure device easily

### Tasks:
- Start AP (RobotConfig)
- Run lightweight web server
- Provide simple config pages
- Save via POST to NVS

### Success Criteria:
- Config accessible via phone
- Changes persist

---

## ✅ Phase 5 – Mode Switch Logic (System Control)

**Goal:** Clean separation of system modes

### Tasks:
- Toggle switch on GPIO (input)
- Use switch to toggle system mode state
- Add serial command to programmatically toggle system mode state
- Add serial output log indicating the system mode state (this will Serialprint once whenever commanded on serial monitor) 
- toggle GPIO(for led indicator) output depending on system mode state
- Monitor continuously
- Add debounce (~200ms)
- Implement mode transitions

### System Modes:
- Config Mode: AP + Web ON, Bluetooth OFF
- Run Mode: WiFi + Bluetooth ON, Web OFF

### Success Criteria:
- Stable switching
- No conflicts

---

## ✅ Phase 6 – API Communication (Cloud Integration)

**Goal:** Send intent data externally

### Tasks:
- Connect WiFi
- Send HTTP POST
- Fire-and-forget (no retry, no queue)
- Send at interval (e.g. 200ms)

### Success Criteria:
- Non-blocking
- No impact on control loop

---

## issue - when toggling back to "mode RUN" the esp32 gets runtime excpetion and reboots
EXCVADDR: 0x00000000  LBEG    : 0x40092b8d  LEND    : 0x40092b9d  LCOUNT  : 0xffffffff  

Backtrace: 0x400914d6:0x3ffe2410 0x401aec67:0x3ffe2440 0x401a7b6e:0x3ffe2460 0x401a7cc3:0x3ffe2770 0x401a4d90:0x3ffe2890 0x401a7d29:0x3ffe2ba0 0x401bc419:0x3ffe2bd0 0x4010d346:0x3ffe2c00 0x4010d3e3:0x3ffe2c30 0x400f4331:0x3ffe2cb0 0x400f43bf:0x3ffe2cf0 0x401bb2eb:0x3ffe2d10 0x401bb368:0x3ffe2d60

ELF file SHA256: 3afbb9d5abe17412

Rebooting...
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0
Bluepad32 (C) 2016-2024 Ricardo Quesada and contributors.
Version: v4.1.0
BTstack: Copyright (C) 2017 BlueKitchen GmbH.
BTstack up and running at 30:AE:A4:FF:16:12
[INFO] System starting

---
## ✅ Phase 7 – Motor Driver & Bench Control

**Goal:** Prove motor control in isolation (no robot yet)

### Tasks:
- Select motor driver (DRV8833, ~~MX1616H,~~ ~~L298N~~, etc.)
- Wire:
  - ESP32 → driver
  - Driver → 1–2 motors
- Implement PWM output
- Test:
  - Forward / reverse
  - Speed control
- Identify:
  - Minimum PWM to move motor (deadband)
  - Direction correctness

### DRV8833 hardware:
- The DRV8833 has 2 inputs per motor:
  - AIN1, AIN2 → Motor A
  - BIN1, BIN2 → Motor B
- then
  - ESP32 GPIO → AIN1
  - ESP32 GPIO → AIN2
  - ESP32 GPIO → BIN1
  - ESP32 GPIO → BIN2
- other pins
  - VM → motor power (e.g. 3–10V)
  - GND → common ground with ESP32
  - VCC → logic power (often 3.3V, ESP32 compatible)
- Truth Table
  - 0	0	Coast
  - 1	0	Forward
  - 0	1	Reverse
  - 1	1	Brake
- GPIO choices (if available)
  - 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
### Example:
```cpp
// basic test
setMotor(LEFT, 100);
delay(2000);
setMotor(LEFT, 0);
```

### Success Criteria:
- Motors respond predictably
- No ESP32 resets
- PWM scaling understood

---

## ✅ Phase 8 – Control Mapping (Intent → Motors)

**Goal:** Integrate your control system with real motor behaviour

### Tasks:
- Map:
```cpp
left  = linear + angular;
right = linear - angular;
```
- Apply:
  - Deadzone (input)
  - Minimum motor threshold (output)
  - Clamping
- Smooth transitions

### Safety:
```cpp
if (noInputFor > 500ms) stopMotors();
```

### Success Criteria:
- Smooth ramping
- No jitter
- Predictable turning

---

## ✅ Phase 9 – Physical Platform (Robot Hardware)

**Goal:** Assemble the full robot AFTER control is proven

### Tasks:
- Mount:
  - Motors
  - Wheels
  - Chassis
  - ESP32
  - Battery
- Clean wiring (short, secure, noise-aware)
- Add physical kill switch
- Balance weight distribution

### Success Criteria:
- Stable movement
- No wiring issues under motion
- No electrical noise causing resets

---

# ✅ BONUS Phase – Controller Abstraction Layer (Multi-Controller Support)

**Goal:** Decouple input system from specific controller hardware (PS4, Xbox, etc.)

### Tasks:
- Create `IController` interface:
  - `begin()`
  - `isConnected()`
  - `read() -> RawInput`
- Refactor existing PS4 logic into `PS4ControllerInput` class
- Implement controller-agnostic `RawInput` mapping
- Add second controller implementation stub (e.g. `XboxControllerInput`)
- Create controller factory / selector:
  - Compile-time flag OR runtime selection
- Ensure all input flows through:
  ```cpp
  RawInput input = controller->read();
  ```
- Remove direct `PS4.*` calls from main loop and other modules

### Architecture:
```
[ PS4 Controller ] ─┐
                    ├──> [ Controller Adapter ] ───> RawInput ───> System
[ Xbox Controller ] ─┘
```

### Success Criteria:
- System runs unchanged when switching controller implementations
- No hardware-specific code outside controller classes
- `RawInput` fully populated regardless of controller type
- Easy to add new controllers without modifying core logic

### Stretch Goals (Optional but Powerful):
- Auto-detect controller type (Bluetooth device name / VID-PID)
- Hot-swapping controllers at runtime
- Add input capability flags (e.g. hasTouchpad, hasGyro)
- Add debug logging:
  - Active controller type
  - Connection events

### ⚠️ Risks / Considerations:
- Different libraries expose different ranges and button naming
- Xbox BLE support on ESP32 is limited compared to PS4
- Some features (e.g. touchpad, gyro) may not map across controllers

---

## 🔧 General Design Decisions

- Typed intent model
- JSON only for config
- Fixed loop timing
- Fire-and-forget networking
- Runtime mode switching
- Logging layer

---

## ⚠️ Things to Watch

### Power Stability
- Separate supplies
- Common ground
- Add capacitor + regulator

### Bluetooth + WiFi
- Keep strict separation

### Controller Compatibility
- Test early

---

## 🚀 Future Expansion

- IMU
- Battery monitoring
- Autonomous modes
- OTA updates
- GPS
- Camera
- Multi-robot systems
- Articulated actuators
- Full telemetry over API
