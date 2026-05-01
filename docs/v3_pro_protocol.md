# Razer BlackShark V3 Pro — HID Protocol

This document captures the wireless dongle's HID command set as decoded from
USBPcap traces of Razer Synapse 4 driving a real V3 Pro (Apr 2026).

The V3 (wired/2.4 GHz, no Pro) and V3 Pro share the same Razer "Kraken-style"
envelope and share most commands; differences are noted inline.

---

## 1. Envelope (90 bytes, HID Report 0x02)

All control writes are sent on the OUT control endpoint as
`URB_FUNCTION_CLASS_INTERFACE` (`bmRequestType=0x21`, `bRequest=9`,
`wValue=0x0202`, `wIndex=5`, `wLength=64`). The 64-byte payload is wrapped in
an additional pseudo-frame yielding 90 captured bytes; the structure of the
report is:

| Offset | Bytes | Meaning |
|--------|-------|---------|
| 0      | 1     | Report ID — always `0x02` |
| 1      | 1     | Status — `0x00` outgoing, set by device on response |
| 2      | 1     | Transaction ID — `0x60` (V3 Pro) |
| 3..4   | 2     | Remaining packets — `0x0000` |
| 5      | 1     | Protocol type — `0x00` |
| 6      | 1     | **Data size** (count of bytes from offset 9 onward that are meaningful) |
| 7..8   | 2     | Reserved — `0x0000` |
| 9      | 1     | **Direction** — `0x80` SET (host→device), `0x00` GET (host queries; device replies in IN URB), `0x84` ACK from device |
| 10     | 1     | **Command class** |
| 11     | 1     | Subclass — always `0x00` for V3 Pro |
| 12     | 1     | **Argument count** — number of meaningful arg bytes (`size - 3`) |
| 13..   | N     | Arguments |
| 88     | 1     | CRC = XOR of bytes `[2..87]` |
| 89     | 1     | Padding `0x00` |

Examples:

```
SET sidetone level = 7:
02 00 60 00 00 00 05 00 00 80 99 00 01 07 00 ... 00 78 00
                          ^size                     ^crc
                                ^dir ^cls ^sub ^cnt^arg

GET battery:
02 00 60 00 00 00 04 00 00 00 21 00 00 00 ... 00 47 00
                          ^size  ^get        ^cnt=00 (no args)
```

---

## 2. Command Classes

### 2.1 State / handshake (`0x00`–`0x03`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x00` | GET | `[]` | Wake / poll handshake |
| `0x01` | GET | `[]` | Status ping (response: `0x84 01`) |
| `0x02` | GET | `[]` | Init handshake (sent on wake) |
| `0x03` | GET | `[]` | Capabilities / serial |

### 2.2 Battery & charging (`0x21`, `0x2a`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x21` | GET | `[]` | Battery level. Response: `[level%, charging?]` |
| `0x2a` | GET | `[]` | Charging state. Response: `[charging? 0/1]` |

V3 driver uses identical commands.

### 2.3 Volume / mic mute readbacks (`0x16`, `0x17`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x16` | GET | `[]` | Mic EQ readback. Response: 10-band table. |
| `0x17` | GET | `[]` | Mic mute state. Response: `[muted 0/1]` |

These are polled by Synapse around every mic-related write.

### 2.4 Audio mode / THX Spatial (`0x9e`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x9e` | SET | `[mode]` | `0x00` = Stereo, `0x01` = THX Spatial |

Switching mode triggers a full re-upload of all 9 EQ slots (see §2.7) — the
firmware EQ tables are mode-dependent.

### 2.5 Active Noise Cancellation / Ambient (`0x92`, `0x12`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x12` | GET | `[]` | Poll current ANC state (Synapse calls this between every set) |
| `0x92` | SET | `[0x02, mode, level]` | Set ANC/ambient |

**Modes:**
- `mode=0x00, level=0x00` — off
- `mode=0x01, level=0x01..0x04` — ANC level 1..4
- `mode=0x50, level=0x01` — Ambient (no levels)

The leading `0x02` arg is the feature selector ("ANC subsystem").

### 2.6 Profile select (`0xe1`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0xe1` | SET | `[profile_idx]` | Activate EQ profile (`0..8`) |

Profiles 0–4 are Default/Game/Movie/Music/Esports (factory). 5–8 are user
custom slots from Synapse's "EDIT EQ LIST" UI.

### 2.7 EQ slot upload (`0xe0`, `0x95`, `0xeb`)

The headphone EQ uses **9 profile slots × 10 bands**. Frequencies (matching the
Synapse UI):

```
31, 63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k Hz
```

Range ±6 dB, sign-magnitude encoded (`0x00` = 0 dB, `0x01..0x06` = +1..+6,
`0x81..0x86` = −1..−6).

**Upload sequence per slot** (5 commands):

```
1. 0xe1 SET [01]                           # gate/start session
2. 0xe0 SET [slot, slot, factory?, 0x00, custom_id, 0x00]   # metadata
3. 0x95 SET [count=0x0b, slot, b0..b9]     # 11-byte payload: profile_idx + 10 bands
4. 0xeb SET [count=0x0b, slot, ...]        # commit
5. 0xe1 SET [02]                           # apply
```

Slot metadata `e0` args observed:
- factory slots 0..4: `[i, i, 01, 00, ?, 00]`
- custom slots 5..8: `[i, i, 00, 00, ?, 00]`

V3 driver previously stored 9 bands at different freqs; **this is wrong** —
both V3 and V3 Pro use 10 bands at the frequencies above.

### 2.8 Mic EQ (`0x96`, `0x97`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x96` | SET | `[0x01, preset_id]` | Select mic EQ preset |
| `0x97` | SET | `[count=0x0a, b0..b9]` | Custom 10-band mic EQ (no profile prefix) |

**Mic presets:**
- `0x20` Default
- `0x21` Esports
- `0x22` Broadcasting
- `0x23` Custom / Flat

The mic EQ has the same 10 bands at the same frequencies as the headphone EQ.

### 2.9 Sidetone (mic monitoring) (`0x98`, `0x99`, `0x2c`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x98` | SET | `[0x01, 0x01]` | Sidetone enable / "begin update" — sent before each level write |
| `0x99` | SET | `[0x01, level]` | Set sidetone level `0x00..0x0f` (0..15) |
| `0x2c` | GET | `[]` | Sidetone level readback |

### 2.10 Wireless power saving (`0xac`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0xac` | SET | `[0x01, minutes]` | Inactivity shutoff timer |

`minutes ∈ {0x00, 0x0f, 0x1e, 0x2d, 0x3c}` = off, 15, 30, 45, 60.

**This confirms the V3 driver's `0xac` guess was correct** — only the arg
order needs fixing (was `[minutes, 0x00]`, should be `[0x01, minutes]`).

### 2.11 Ultra-Low Latency (`0xdf`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0xdf` | SET | `[0x01, on]` | Toggle Ultra-Low Latency mode (`0`/`1`) |

Smoke-tested on V3 (audible link renegotiation when toggled). Driver had it as
the THX class — this is wrong; THX is `0x9e`. `0xdf` is exclusively ULL.

### 2.12 In-call audio mix (`0xdd`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0xdd` | SET | `[0x01, mode]` | BT-call behaviour while on 2.4 GHz |

**Modes:**
- `0x00` — Combine 2.4 GHz and Bluetooth
- `0x01` — Lower 2.4 GHz volume
- `0x02` — Mute 2.4 GHz volume

Present on both V3 and V3 Pro.

### 2.13 Game / Chat balance (`0xdc`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0xdc` | SET | `[0x01, balance]` | `0x00..0x14` (0..20). 0 = full chat, 10 = center, 20 = full game |

Present on both V3 and V3 Pro.

### 2.14 Audio prompts (`0xe5`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0xe5` | SET | `[0x02, 0x00, on]` | Voice-prompt toggle (mic mute/unmute prompt) |

Pushed to V3 in a recent firmware update — present on both V3 and V3 Pro.

### 2.15 Bulk EQ readback (`0x60`, `0x15`)

| Class | Direction | Args | Meaning |
|-------|-----------|------|---------|
| `0x60` | SET | `[0x01, slot]` | Request EQ slot N metadata |
| `0x15` | SET | `[0x01, slot]` | Request EQ slot N bands |

Synapse iterates slots `0..8` at startup to populate the EQ list. Only needed
for "read back what's stored on the headset" — not required for SET-side
control, but useful for state restoration after reboot.

### 2.16 Indicator LED & misc (TBD)

The dongle LED-mode UI (Connection / Battery / Battery-Warning) is not in any
captured pcap. Likely uses a dedicated class around `0x55..0x6a` (these
appeared once in the baseline startup capture, so they're enumerated at boot).
Needs a dedicated capture.

The Audio Enhancement panel (Sound Normalization / Bass Boost / Voice Clarity
+ intensity slider) is also uncaptured. Same holds for the Mic Enhancement
modes (Volume Normalization / Vocal Clarity / Mic NC / Voice Gate).

---

## 3. Direction byte semantics

| Byte | Meaning | Example use |
|------|---------|-------------|
| `0x00` | GET (host queries device) | `0x21` battery, `0x16` mic EQ readback |
| `0x80` | SET (host writes value) | All toggles, EQ writes, mode switches |
| `0x84` | ACK (device → host response) | Returned in IN URB; payload contains current value |

---

## 4. Argument-count byte (`buf[12]`)

For all `0x80` SET writes, `buf[12]` equals the number of meaningful arg bytes
following it. Equivalent to `(size - 3)` where `size = buf[6]`.

Examples:
- `99 00 01 [level]` — 1 arg
- `92 00 02 [mode] [level]` — wait: actual is `02 [mode] [level]` = 3 bytes,
  so `count=02` is the *feature selector* not arg count for this class.
  **For most classes count == arg count**, but `0x92` and a few others encode
  a feature/sub-selector in this slot. When in doubt: `count = size - 3` always
  holds; the *interpretation* of args is per-class.

---

## 5. CRC

`buf[88] = XOR of buf[2..87]`. Trailing byte `buf[89]` is `0x00`.

Reference implementation already in `razercommon.c::razer_calculate_crc()`.

---

## 6. Source captures

All pcaps in `~/v3pro-captures/v3pro/Pcapv3pro/` (USBPcap from Razer Synapse 4
on Windows, captured Apr 29 2026):

| Pcap | Decoded |
|------|---------|
| `vp_01_baseline_startup.pcapng` | Handshake + capability scan |
| `vp_02_headphone_eq.pcapng` | 9-slot upload + profile cycle |
| `vp_03_mic_eq.pcapng` | Per-band sweeps, presets `0x21`/`0x22`, reset |
| `vp_04_sidetone.pcapng` | Levels 0..15 |
| `vp_06_ancandambient.pcapng` | ANC off→1..4→ambient→off |
| `vp_07_power_save.pcapng` | off→on→15→30→45→60→off |
| `vp_09_battery_charge_cycle.pcapng` | Plug/unplug events |
| `Audio Prompts on to off again.pcapng` | `0xe5` toggle |
| `GameChat Mixer.pcapng` | `0xdc` slider 0..20 |
| `incall audio mix.pcapng` | `0xdd` modes 0/1/2 |
| `stereotothxtostereo.pcapng` | Mode + full re-upload |
| `Ultra-Lowlatencyonoff.pcapng` | `0xdf` toggle |
| `Mic back to default and clicked reset.pcapng` | `0x96 [01,20]` + custom band reset |

---

## 7. Open questions / TBD

1. **Indicator LED mode** — needs a dedicated capture (toggle through
   Connection / Battery / Battery-Warning).
2. **Audio Enhancement** (Sound Normalization / Bass Boost / Voice Clarity +
   intensity slider) — uncaptured.
3. **Mic Enhancement** (Volume Normalization / Vocal Clarity / Mic NC / Voice
   Gate + intensity slider) — uncaptured.
4. **Microphone volume** (0..100 slider on MIC tab) — uncaptured. Likely a
   distinct class from mic mute (`0x17`).
5. **Firmware version readback** — `0x80` baseline scan touched several
   `0x55..0x6a` classes; mapping not yet determined.
6. **`0x98`** sidetone always sent with args `[0x01, 0x01]` paired with each
   `0x99` write. Possibly a "session begin" or "enable monitoring" but always
   written before any level change. Treat as required precursor for now.
