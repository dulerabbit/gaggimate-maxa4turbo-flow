# gaggimate-maxa4turbo-flow

Community fork of [GaggiMate](https://github.com/jniebuhr/gaggimate) with **Digmesa measured-flow closed-loop** for Pro + Velofuso.

**Not an official GaggiMate / Jochen release.** Built for [#bldc-pumps](https://discord.com) / maxA4turbo-style Digmesa setups.

## Flash (easiest)

**Web flasher (Chrome / Edge):**  
**https://dulerabbit.github.io/gaggimate-maxa4turbo-flow/**

1. Open that page on a desktop Chrome or Edge browser  
2. Plug in the **Pro controller** USB  
3. Click **Connect & Install**  
4. Leave your **display** alone  

## Digmesa wiring

| Digmesa | Pro header |
|---------|------------|
| Signal (open collector) | **EXT4 → GPIO12** |
| GND | GND |
| +V | 3.3–5 V |

Add an external **pull-up to 3.3 V** (~1k–4.7k). Do **not** use EXT2 (I2C SCL), Velofuso TCA IO2, or GPIO14.

## Alternate: Release bins + esptool

Download assets from the latest [Release](https://github.com/dulerabbit/gaggimate-maxa4turbo-flow/releases):

| File | Role |
|------|------|
| `board-bootloader.bin` | bootloader @ `0x0` |
| `board-partitions.bin` | partition table @ `0x8000` |
| `board-firmware.bin` | app @ `0x10000` |

```bash
esptool.py --chip esp32s3 --port /dev/tty.usbmodem* write_flash \
  0x0     board-bootloader.bin \
  0x8000  board-partitions.bin \
  0x10000 board-firmware.bin
```

Or from source: `pio run -e controller -t upload`

## Behaviour

- When Velofuso (TCA @ `0x26`) is detected and `GM_MEASURED_FLOW=1` (default in this fork’s `controller` env), FLOW mode runs a **PI on measured ml/s**.
- Pressure target in the same command is still a **ceiling** (`min(flowPI, pressureDuty)`).
- Telemetry `pump_flow` / volumetric come from the meter.
- Digmesa 1.2 mm default K-factor ≈ **31 100 pulses/L** (`0.03215 ml/pulse`). Tune in `FlowSensor.h` if needed.

Stock model path: build `-e controller-stock` (`GM_MEASURED_FLOW=0`).

## Source base

- Upstream: `jniebuhr/gaggimate` `master`
- Patch: Digmesa measured-flow PI on Pro EXT4

## License

Same as upstream GaggiMate.
