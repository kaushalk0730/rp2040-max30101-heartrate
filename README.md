# Post-Exercise Syncope Guard

**A wearable early-warning system for post-exertional near-fainting**

---

## Problem Statement

When people abruptly stop intense exercise (e.g., stopping a sprint and standing still), blood
that was being actively pumped back to the heart by the leg muscles suddenly pools in the lower
body. This causes a rapid drop in blood pressure, leading to **near-fainting** — dizziness,
graying/blacking of vision, and fading in and out of consciousness.

## The Solve

A small wearable device that continuously monitors the wearer's heart rate and movement. It
watches for the specific signature of this event — **a period of high cardiovascular exertion
followed by a sudden, abrupt stop in movement** — and proactively warns the user before symptoms
of near-fainting set in.

When that pattern is detected, the device alerts the user with guidance to:
- Slowly walk it off rather than standing still, **or**
- Lie down on their back

---

## High-Level Architecture

- **Human Panel** — where the sensor sits against the skin (finger or wrist).
- **Sensor (MAX30101)** — reads heart rate (and, once integrated, motion) from the body.
- **Microcontroller (RP2040)** — reads the sensor, figures out if the risky pattern is
  happening, and reports the result.
- **Serial Monitor** — where the output (heart rate, alerts) is shown for now during
  development.

---

## Low-Level Flow

1. Human puts their finger on the sensor.
2. The sensor sends that info to the microcontroller via I2C.
3. The microcontroller computes the info and sends the output (heart rate) to a serial monitor.

*(Once motion sensing is integrated, a similar flow will feed in movement data so the
microcontroller can watch for the abrupt-stop pattern too.)*

---

## Implementation Status

- ✅ **Heart rate monitoring** — working. The device can already detect the exertion spike.
- 🚧 **Motion sensing** — in progress. This is the piece needed to detect the abrupt stop. Until
  it's finished, the device can see the exertion half of the pattern but can't yet confirm the
  stop, so the full warning isn't live yet.

---

## Status / Next Steps

- [ ] **Finish motion sensing integration** — top priority; unlocks the abrupt-stop half of
      detection
- [ ] Tune thresholds for what counts as an "exertion spike" and an "abrupt stop" using real
      test data
- [ ] Avoid false alarms from momentary pauses (e.g., tying a shoe)
- [ ] Decide how the device alerts the user (vibration, sound, phone notification)
