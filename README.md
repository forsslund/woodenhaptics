# WoodenHaptics

![Open Source Hardware](oshw-logo.svg)

**A starting kit for crafting force-reflecting spatial haptic devices** — an
open-source-hardware haptic device you can build yourself from lasercut
plywood, off-the-shelf motors, and open electronics. First presented at ACM
TEI 2015 ([paper PDF](woodenhaptics-tei-2015-paper.pdf)); more at
[woodenhaptics.org](http://woodenhaptics.org).

> **This is the official WoodenHaptics repository.** It carries the full git
> history (2015→today) of
> [`WoodenHaptics/TEI_2015`](https://github.com/WoodenHaptics/TEI_2015), which
> remains available as the frozen, citable artifact of the TEI 2015 paper.

## What's here

| | |
|---|---|
| `Mechanics/`, `Electronics/`, `Software/`, `Docs/`, BOM v1.0 | The **TEI-2015 reference design** (RE40 motors) — the citable original |
| [`2017-edition/`](2017-edition/) | The **2017 kit revision** (RE30 motors, updated lasercut + BOM v1.1) with its own provenance README |
| [`firmware/woody_teensy_mini/`](firmware/woody_teensy_mini/) | **Current firmware** — Teensy 4.0 on the 2019 adapter board (the 2026 way to run the device) |
| [`2026-consolidation-plan.md`](2026-consolidation-plan.md), [`freecad-migration-plan.md`](freecad-migration-plan.md) | Ongoing work: repo restructuring and an open parametric FreeCAD model |

## Running it in 2026 — the software stack

The device electronics have evolved from DAQ (2015) via mbed-USB (2017) to a
**Teensy 4.0** adapter (2019→). The current stack, all open:

| Layer | Where |
|---|---|
| Mechanics, BOM, electronics, firmware | **this repo** |
| PC driver / API (kinematics, forces) | [`forsslund/hfabapi`](https://github.com/forsslund/hfabapi) |
| Haptic rendering | [`forsslund/chai3d`](https://github.com/forsslund/chai3d/tree/using_hfabapi), branch `using_hfabapi` |
| Demo application | [`forsslund/duohaptics`](https://github.com/forsslund/duohaptics) |

Plug in the USB, flash `firmware/woody_teensy_mini/`, build hfabapi + the
chai3d branch, and the device runs on a modern Linux or Windows machine.

## A short history

- **2012** — first prototype at Stanford University
- **2015** — ACM TEI paper, demo, and this open-source kit (with Michael Yip and Jordi Solsona Belenguer)
- **2017** — WorldHaptics demo ("WoodenHaptics 1.5"), kit edition with RE30 motors and USB electronics
- **2019** — Teensy adapter board; the mbed/DAQ paths retire
- **2026** — revival: firmware/API cleanup, FreeCAD migration, EuroHaptics open-source-hardware workshop

Older experimental material (mbed code, PCB v0.4 sources, chai3d 3.0 fork)
lives in [`WoodenHaptics/woody-experimental`](https://github.com/WoodenHaptics/woody-experimental)
(historical, do not use).

## Sibling projects

The open controller approach born here grew into
[**Polhem**](https://github.com/forsslund/polhem) (our flagship device) and
[**vintagehaptics**](https://github.com/forsslund/vintagehaptics) — reviving
classic SensAble PHANTOM devices with the same open stack.

## License & contact

**CC-BY-SA 4.0** unless noted otherwise (CHAI3D extensions are BSD, like their
parent project). For other licensing or kit distribution:
Jonas Forsslund — jonas.forsslund@gmail.com

/ Jonas, Mike and Jordi
