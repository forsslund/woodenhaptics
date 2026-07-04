# woody_teensy_mini — WoodenHaptics firmware (Teensy 4.0)

The firmware for driving WoodenHaptics with the **Teensy 4.0 adapter board**
(`teensy_adapter_v1_20191008`) on the WoodenHaptics v0.4 PCB. This is the
firmware used by the 2026 revival of the project.

**Provenance:** derived from
[`forsslund/polhem`](https://github.com/forsslund/polhem)
`firmware/polhem_teensy_mini` — the same protocol and structure, with
Woody-specific pin mapping, software (interrupt-based) encoders, and its own
device id (`model_woody_raw = 3`). This copy is the canonical home for the
Woody variant; Polhem's own firmware lives in the polhem repo.

## Protocol

Plain-text serial over USB at ~1 kHz:
device → PC `[model,enc0,enc1,enc2,enc3,enc4,enc5,error]\n` with `model = 3`;
PC → device sets motor currents. Kinematics are computed on the PC by
[`forsslund/hfabapi`](https://github.com/forsslund/hfabapi), which selects the
WoodenHaptics kinematic configuration when it sees model 3.

Motor amps are enabled while servo messages flow and cut by a message
watchdog — no separate power-enable switch is used.

## Build & flash

Arduino IDE / arduino-cli with Teensyduino, target **Teensy 4.0**. Uses the
stock `Encoder` and `Bounce2` libraries.

## Hardware notes

- Enable signal runs on **Teensy pin 1** (jumper wire on the adapter board);
  pin 14 is unusable for this (pulled by a transceiver on the adapter).
- Do **not** add `QuadEncoder.h` (Teensy 4.x hardware quadrature library):
  merely including it reconfigures the XBAR peripheral at startup and hijacks
  pin 14. Woody uses software encoders on interrupt pins.
