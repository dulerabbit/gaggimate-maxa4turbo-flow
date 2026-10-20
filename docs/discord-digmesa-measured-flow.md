# Digmesa measured-flow patch (for maxA4turbo / #bldc-pumps)

Community fork patch. **Not** an official GaggiMate / Jochen release.

## What this does

- Wires a pulse flow meter (Digmesa Nano Brass **9NB-0120-01A**) to GaggiMate Pro **EXT4 → GPIO12**
- Counts pulses with a hardware IRQ
- In **FLOW** mode: PI on **measured ml/s** (no pump-model guessing)
- Pressure target from the same pump-control message is still a **ceiling** via `min(flowPI, pressureDuty)`
- Telemetry `pump_flow` / volumetric / `water_pumped` come from the meter when enabled

## Wiring

| Digmesa | Pro |
|---------|-----|
| Signal (OC) | **EXT4** (GPIO12) |
| GND | GND |
| +V | 3.3–5 V (Digmesa: 2.8–24 V) |

Digmesa is **open collector** — needs a **pull-up to 3.3 V** (~1k–4.7k). The firmware also enables the ESP internal pull-up; an external pull-up is still recommended for long cables.

**Do not** use EXT2 (I2C SCL), Velofuso TCA IO2, or “GPIO14”.

## Apply the patch

Base: **upstream GaggiMate `master`** (commit noted in the patch header).

```bash
cd gaggimate   # your checkout of https://github.com/jniebuhr/gaggimate
git apply /path/to/0001-feat-controller-Digmesa-measured-flow-on-Pro-EXT4.patch
# or: git am 0001-....patch
```

Then build/flash **controller only**:

```bash
pio run -e controller -t upload
```

Default `controller` env has `-DGM_MEASURED_FLOW=1`. To keep stock model flow:

```bash
pio run -e controller-stock -t upload
```

## Calibration

Default K-factor: Digmesa 1.2 mm **without** pulse divider ≈ **31 100 pulses/L** → **0.03215 ml/pulse**.

If your install reads high/low, change `DIGMESA_NB_0120_ML_PER_PULSE` in `FlowSensor.h` or call `setMlPerPulse()` after setup.

Starter PI gains (in `PressureController`): **Kp=8**, **Ki=4**. If it oscillates, lower both; if laggy, raise Ki carefully. You can also call `DimmedPump::setMeasuredFlowGains(kp, ki)`.

## Expected behaviour

1. Boot with Velofuso (TCA @ 0x26) → log: `GM_MEASURED_FLOW: FlowSensor on EXT4`
2. FLOW profile → pump power tracks measured flow; `pump_flow` on the display should match the meter, not the curve
3. Blind/choked puck → pressure ceiling bites; flow PI does not wind forever
4. No pulses → measured flow goes to 0 within ~400 ms

## What this does *not* change

- Display firmware / UI
- Velofuso DAC path (still MCP4725 via SoftWire on expander IO0/IO1)
- Pressure / POWER modes (still stock)

## Feedback

If something fails, note: pulse Hz at a known flow, whether pressure clamp works, serial log lines around `FlowSensor` / `GM_MEASURED_FLOW`.
