# 🎮 How to Connect a PS4 Controller to ESP32

This guide will walk you through connecting a PlayStation 4 (DualShock 4) controller to your ESP32 using Bluetooth.

---

## ✅ What You Need

- ESP32 (powered via USB)
- PS4 Controller (DualShock 4)
- USB cable (for ESP32)
- Computer with Serial Monitor (e.g. VS Code / PlatformIO)

---

## 🚀 Step 1: Upload Your Code

Make sure your project includes:

- PS4 Controller library
- `PS4.begin()` inside `initInput()`

Then:

1. Connect ESP32 to your computer via USB  
2. Upload your project  
3. Open Serial Monitor (115200 baud)

---

## 🔵 Step 2: Start Bluetooth

After upload, you should see:

```
PS4 Bluetooth ready. Hold PS + Share to pair.
```

This means the ESP32 is ready.

---

## 🎮 Step 3: Put Controller in Pairing Mode

On your PS4 controller:

1. Press and hold:
   - **PS button**
   - **SHARE button**

2. Hold for ~5 seconds

3. The light bar will start **flashing rapidly**

👉 This means it's in pairing mode

---

## 🔗 Step 4: Wait for Connection

Within a few seconds, you should see:

```
PS4 Connected!
```

And your logs should show:

```
[INFO] Controller state: Connected
```

---

## 🧪 Step 5: Test Input

Move the sticks or press buttons.

You should see logs like:

```
Intent L: 0.45 A: -0.12 Conn: 1
```

---

## 🔄 Reconnecting Later

After first pairing:

- Controller will auto-reconnect when turned on
- No need to repeat pairing steps

---

## ⚠️ Troubleshooting

### ❌ Controller not connecting
- Hold **PS + SHARE** again
- Reset ESP32
- Ensure controller is not connected to another device

---

### ❌ No input values changing
- Check `readInput()` implementation
- Confirm normalization is correct
- Verify correct library functions

---

### ❌ Random disconnects
- Try a different USB cable
- Use a rear USB port on PC
- Ensure stable power supply

---

### ❌ ESP32 resets (brownout)
- Use better power source
- Avoid weak USB hubs

---

## 🧠 Notes

- ESP32 acts as the Bluetooth host (like a console)
- Your computer is only used for power + logging
- All control logic runs on the ESP32

---

## ✅ You're Ready!

Once connected, your controller feeds into:

```
Controller → RawInput → Intent → Robot
```

Happy building 🚀