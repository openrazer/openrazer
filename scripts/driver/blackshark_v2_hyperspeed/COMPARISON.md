# Existing work compared before submission

Comparison against current OpenRazer master `477f4d6d3ab7e9deb6310d5bd3742560c5cbe80d`
and the following fetched proposals. No commits were cherry-picked from them.

| Work | Overlap and resulting decision |
| --- | --- |
| AlexandreFournier `blackshark-v2-hyperspeed`, `9f3a3ac2` | Already implements both V2 HyperSpeed PIDs and the main onboard controls. Credit this existing work rather than presenting basic support as new discovery. Adopt its measured reply-status interpretation from `d7067eb1`: byte 1 is status, success 2 and unsupported 5. Preserve our stricter sequence, checksum, length and domain matching, awaited setter replies, input validation, receiver/headset identity separation, and EQ latch sequence. |
| #2862, V2 Pro `1532:0555` | Different MXIC protocol and a different device. Also proposes a `razerblackshark` module and string-valued presets on `razer.device.audio`. Use a V2-specific module and interface to avoid name/signature collisions. Do not claim V2 Pro support. |
| #2794, V3/V3 Pro | Has additional V3-specific protocol, telemetry, endpoint and RF-wake behavior. Do not import its private URB, keep-alive, USB enumeration quirks or unverified controls into this V2 patch. V2 works through normal HID input on the tested firmware. |
| #2899, V3 X | Different report ID and layout; also proposes `razerblackshark`. Name our module `razerblacksharkv2`. Its same-serial, offline-write and reconnect observations inform separate tests; they do not prove V2 behavior. |
| #2868, battery power_supply export | Complementary shared infrastructure. Keep the core device-support patch independent and do not duplicate its battery helper. Test/report the existing mouse path separately. |

Alexandre's implementation also disables autosuspend for the shared USB device,
uses fallback metadata until identity reads succeed, and retries identity lazily.
We retain the real, stable receiver identity, avoid invented metadata, and leave
USB autosuspend policy unchanged unless a reproduced V2 failure requires it.
Neither architecture alone proves sleep/reconnect safety; those are hardware
acceptance gates.

The new Python device class is selected only for VID `1532`, PIDs `0565/056E`.
Existing headsets retain their original factory behavior. The proposed audio API
uses `razer.device.audio.blacksharkv2`, so existing proposals' incompatible preset
signatures cannot collide. API consolidation remains a maintainer review topic.

References:

- https://github.com/AlexandreFournier/openrazer/tree/blackshark-v2-hyperspeed
- https://github.com/AlexandreFournier/openrazer/commit/d7067eb1
- https://github.com/openrazer/openrazer/pull/2862
- https://github.com/openrazer/openrazer/pull/2794
- https://github.com/openrazer/openrazer/pull/2899
- https://github.com/openrazer/openrazer/pull/2868
