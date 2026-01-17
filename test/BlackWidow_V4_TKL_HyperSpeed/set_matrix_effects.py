#!/usr/bin/env python3
import argparse
import glob
import os
import sys
import time
from dataclasses import dataclass


@dataclass(frozen=True)
class Device:
    root: str

    def path(self, name: str) -> str:
        return os.path.join(self.root, name)

    def exists(self, name: str) -> bool:
        return os.path.exists(self.path(name))


EFFECT_NODES = [
    'matrix_effect_none',
    'matrix_effect_spectrum',
    'matrix_effect_wave',
    'matrix_effect_static',
    'matrix_effect_blinking',
    'matrix_effect_reactive',
    'matrix_effect_breath',
    'matrix_effect_starlight',
    'matrix_effect_custom',
]


def parse_int(name: str, value: object, *, min_value: int | None = None, max_value: int | None = None) -> int:
    try:
        i = int(value)
    except (TypeError, ValueError):
        raise SystemExit(f"{name} must be an integer")

    if min_value is not None and i < min_value:
        raise SystemExit(f"{name} must be >= {min_value}")
    if max_value is not None and i > max_value:
        raise SystemExit(f"{name} must be <= {max_value}")
    return i


def parse_rgb(s: str) -> tuple[int, int, int]:
    parts = [int(x) for x in s.split(',')]
    if len(parts) != 3 or not all(0 <= x <= 255 for x in parts):
        raise ValueError
    return parts[0], parts[1], parts[2]


def enable_custom(dev: Device) -> None:
    p = require_node(dev, 'matrix_effect_custom')
    write_bytes(p, b'1')


def require_frame_node(dev: Device) -> str:
    p = dev.path('matrix_custom_frame')
    if not os.path.exists(p):
        raise SystemExit(f"This device does not expose matrix_custom_frame: {p}")
    return p


def frame_write_row(dev: Device, row: int, start_col: int, stop_col: int, rgb_bytes: bytes) -> None:
    frame_path = require_frame_node(dev)
    payload = bytes([row, start_col, stop_col]) + rgb_bytes
    write_bytes(frame_path, payload)
    enable_custom(dev)


def solid_rgb_bytes(cols: int, rgb: tuple[int, int, int]) -> bytes:
    r, g, b = rgb
    return bytes([r, g, b]) * cols


def checker_rgb_bytes(cols: int, rgb_a: tuple[int, int, int], rgb_b: tuple[int, int, int], row: int) -> bytes:
    ra, ga, ba = rgb_a
    rb, gb, bb = rgb_b
    out = bytearray()
    for col in range(cols):
        use_a = ((row + col) % 2) == 0
        if use_a:
            out += bytes([ra, ga, ba])
        else:
            out += bytes([rb, gb, bb])
    return bytes(out)


def list_device_dirs(pattern: str) -> list[str]:
    candidates = [p for p in glob.glob(pattern) if os.path.isdir(p)]
    candidates.sort()
    return candidates


def pick_device_dir(pattern: str) -> str:
    candidates = list_device_dirs(pattern)
    if not candidates:
        raise SystemExit(f"No razerkbd devices found for: {pattern}")

    # Prefer a directory that clearly looks like a keyboard LED interface.
    for preferred_node in ('matrix_effect_static', 'matrix_effect_custom', 'matrix_custom_frame'):
        for root in candidates:
            if os.path.exists(os.path.join(root, preferred_node)):
                return root

    return candidates[0]


def write_text(path: str, value: str) -> None:
    try:
        with open(path, 'w') as f:
            f.write(value)
    except PermissionError as e:
        raise SystemExit(f"Permission denied writing {path}. Try running with sudo. ({e})")


def write_bytes(path: str, payload: bytes) -> None:
    try:
        with open(path, 'wb') as f:
            f.write(payload)
    except PermissionError as e:
        raise SystemExit(f"Permission denied writing {path}. Try running with sudo. ({e})")


def require_node(dev: Device, node: str) -> str:
    p = dev.path(node)
    if not os.path.exists(p):
        available = [n for n in EFFECT_NODES if dev.exists(n)]
        raise SystemExit(
            f"This device does not expose {node}: {p}\n"
            f"Available nodes: {', '.join(available) if available else '(none found)'}"
        )
    return p


def cmd_list(dev: Device) -> int:
    print('Device:', dev.root)
    found = False
    for node in sorted(set(EFFECT_NODES + ['matrix_custom_frame', 'matrix_current_effect', 'matrix_reactive_trigger'])):
        p = dev.path(node)
        if os.path.exists(p):
            found = True
            print(f"- {node}: {p}")
    if not found:
        print('No known matrix nodes found under this device directory.')
    return 0


def cmd_none(dev: Device) -> int:
    p = require_node(dev, 'matrix_effect_none')
    print('matrix_effect_none:', p)
    write_text(p, '1')
    return 0


def cmd_spectrum(dev: Device) -> int:
    p = require_node(dev, 'matrix_effect_spectrum')
    print('matrix_effect_spectrum:', p)
    write_text(p, '1')
    return 0


def cmd_wave(dev: Device, direction: int) -> int:
    p = require_node(dev, 'matrix_effect_wave')
    # Daemon uses 1 (L->R) or 2 (R->L). Clamp to 1/2.
    if direction not in (1, 2):
        direction = 1
    print('matrix_effect_wave:', p)
    write_text(p, str(direction))
    return 0


def cmd_static(dev: Device, rgb: tuple[int, int, int]) -> int:
    p = require_node(dev, 'matrix_effect_static')
    r, g, b = rgb
    print('matrix_effect_static:', p)
    write_bytes(p, bytes([r, g, b]))
    return 0


def cmd_blinking(dev: Device, rgb: tuple[int, int, int]) -> int:
    p = require_node(dev, 'matrix_effect_blinking')
    r, g, b = rgb
    print('matrix_effect_blinking:', p)
    write_bytes(p, bytes([r, g, b]))
    return 0


def cmd_reactive(dev: Device, rgb: tuple[int, int, int], speed: int) -> int:
    p = require_node(dev, 'matrix_effect_reactive')
    # Daemon clamps speed to 1..4, defaults to 4.
    if speed not in (1, 2, 3, 4):
        speed = 4
    r, g, b = rgb
    print('matrix_effect_reactive:', p)
    write_bytes(p, bytes([speed, r, g, b]))
    return 0


def cmd_breath_random(dev: Device) -> int:
    p = require_node(dev, 'matrix_effect_breath')
    print('matrix_effect_breath (random):', p)
    # Matches daemon: payload = b'1'
    write_bytes(p, b'1')
    return 0


def cmd_breath_single(dev: Device, rgb: tuple[int, int, int]) -> int:
    p = require_node(dev, 'matrix_effect_breath')
    r, g, b = rgb
    print('matrix_effect_breath (single):', p)
    write_bytes(p, bytes([r, g, b]))
    return 0


def cmd_breath_dual(dev: Device, rgb1: tuple[int, int, int], rgb2: tuple[int, int, int]) -> int:
    p = require_node(dev, 'matrix_effect_breath')
    r1, g1, b1 = rgb1
    r2, g2, b2 = rgb2
    print('matrix_effect_breath (dual):', p)
    write_bytes(p, bytes([r1, g1, b1, r2, g2, b2]))
    return 0


def cmd_breath_triple(dev: Device, rgb1: tuple[int, int, int], rgb2: tuple[int, int, int], rgb3: tuple[int, int, int]) -> int:
    p = require_node(dev, 'matrix_effect_breath')
    r1, g1, b1 = rgb1
    r2, g2, b2 = rgb2
    r3, g3, b3 = rgb3
    print('matrix_effect_breath (triple):', p)
    write_bytes(p, bytes([r1, g1, b1, r2, g2, b2, r3, g3, b3]))
    return 0


def cmd_starlight_random(dev: Device, speed: int) -> int:
    p = require_node(dev, 'matrix_effect_starlight')
    speed = max(0, min(255, int(speed)))
    print('matrix_effect_starlight (random):', p)
    write_bytes(p, bytes([speed]))
    return 0


def cmd_starlight_single(dev: Device, rgb: tuple[int, int, int], speed: int) -> int:
    p = require_node(dev, 'matrix_effect_starlight')
    speed = max(0, min(255, int(speed)))
    r, g, b = rgb
    print('matrix_effect_starlight (single):', p)
    write_bytes(p, bytes([speed, r, g, b]))
    return 0


def cmd_starlight_dual(dev: Device, rgb1: tuple[int, int, int], rgb2: tuple[int, int, int], speed: int) -> int:
    p = require_node(dev, 'matrix_effect_starlight')
    speed = max(0, min(255, int(speed)))
    r1, g1, b1 = rgb1
    r2, g2, b2 = rgb2
    print('matrix_effect_starlight (dual):', p)
    write_bytes(p, bytes([speed, r1, g1, b1, r2, g2, b2]))
    return 0


def cmd_custom(dev: Device) -> int:
    p = require_node(dev, 'matrix_effect_custom')
    print('matrix_effect_custom:', p)
    write_bytes(p, b'1')
    return 0


def cmd_frame_pixel(dev: Device, row: int, col: int, rgb: tuple[int, int, int], hold: float, clear: bool) -> int:
    frame_path = require_frame_node(dev)
    r, g, b = rgb
    print('matrix_custom_frame:', frame_path)
    enable_custom(dev)

    # payload: ROW_ID, START_COL, STOP_COL, R, G, B (START=STOP for 1 pixel)
    write_bytes(frame_path, bytes([row, col, col, r, g, b]))
    enable_custom(dev)

    if hold > 0:
        time.sleep(hold)

    if clear:
        write_bytes(frame_path, bytes([row, col, col, 0, 0, 0]))
        enable_custom(dev)

    return 0


def cmd_frame_fill(
    dev: Device,
    rows: int,
    cols: int,
    pattern: str,
    rgb: tuple[int, int, int],
    rgb2: tuple[int, int, int],
    hold: float,
    clear: bool,
) -> int:
    frame_path = require_frame_node(dev)
    print('matrix_custom_frame:', frame_path)
    enable_custom(dev)

    start_col = 0
    stop_col = cols - 1
    print(f'Filling {rows}x{cols} pattern={pattern} hold={hold}s clear={clear}')

    for row in range(rows):
        if pattern == 'solid':
            rgb_bytes = solid_rgb_bytes(cols, rgb)
        else:
            rgb_bytes = checker_rgb_bytes(cols, rgb, rgb2, row)
        frame_write_row(dev, row, start_col, stop_col, rgb_bytes)

    if hold > 0:
        time.sleep(hold)

    if clear:
        clear_bytes = solid_rgb_bytes(cols, (0, 0, 0))
        for row in range(rows):
            frame_write_row(dev, row, start_col, stop_col, clear_bytes)

    return 0


def cmd_frame_scan(dev: Device, rows: int, cols: int, rgb: tuple[int, int, int]) -> int:
    frame_path = require_frame_node(dev)
    print('matrix_custom_frame:', frame_path)
    enable_custom(dev)

    r, g, b = rgb
    visible: list[tuple[int, int]] = []

    print(f"\nScanning {rows}x{cols} (rows=0..{rows-1}, cols=0..{cols-1})")
    print("Tip: open another terminal and run: sudo dmesg -w")

    def ask_visible() -> str:
        return input("Did you see anything light up? [y/N/q] ").strip().lower()

    for row in range(rows):
        for col in range(cols):
            print(f"\nrow={row} col={col}")
            write_bytes(frame_path, bytes([row, col, col, r, g, b]))
            enable_custom(dev)

            try:
                resp = ask_visible()
            except EOFError:
                resp = ''

            # turn off the same pixel
            write_bytes(frame_path, bytes([row, col, col, 0, 0, 0]))
            enable_custom(dev)

            if resp == 'q':
                row = rows
                break
            if resp == 'y':
                visible.append((row, col))

    if not visible:
        print("\nSummary: you did not mark any coordinates as visible.")
        return 0

    seen_rows = sorted({rr for rr, _ in visible})
    seen_cols = sorted({cc for _, cc in visible})
    max_row = max(seen_rows)
    max_col = max(seen_cols)

    print("\nSummary:")
    print("- Visible coordinates:", ' '.join([f"({rr},{cc})" for rr, cc in visible]))
    print("- Rows with visibility:", seen_rows)
    print("- Columns with visibility:", seen_cols)
    print(f"- Max row/col seen: row={max_row} col={max_col}")
    print(f"- Suggested MIN (bounding box): rows={max_row + 1} cols={max_col + 1} (assuming origin at 0)")

    return 0


def cmd_frame_animate(
    dev: Device,
    rows: int,
    cols: int,
    pattern: str,
    rgb: tuple[int, int, int],
    rgb2: tuple[int, int, int],
    fps: float,
    seconds: float,
    clear: bool,
) -> int:
    frame_path = require_frame_node(dev)
    print('matrix_custom_frame:', frame_path)
    enable_custom(dev)

    if fps <= 0:
        raise SystemExit('--fps must be > 0')
    if seconds <= 0:
        raise SystemExit('--seconds must be > 0')

    frame_time = 1.0 / fps
    total_frames = max(1, int(round(seconds * fps)))
    start_col = 0
    stop_col = cols - 1

    def clear_all() -> None:
        clear_bytes = solid_rgb_bytes(cols, (0, 0, 0))
        for rr in range(rows):
            frame_write_row(dev, rr, start_col, stop_col, clear_bytes)

    try:
        if pattern == 'snake':
            # One pixel moving in row-major order, alternating direction per row.
            path: list[tuple[int, int]] = []
            for rr in range(rows):
                if rr % 2 == 0:
                    for cc in range(cols):
                        path.append((rr, cc))
                else:
                    for cc in range(cols - 1, -1, -1):
                        path.append((rr, cc))

            for i in range(total_frames):
                rr, cc = path[i % len(path)]
                # Turn on current pixel
                r, g, b = rgb
                write_bytes(frame_path, bytes([rr, cc, cc, r, g, b]))
                enable_custom(dev)

                # Turn off previous pixel
                pr, pc = path[(i - 1) % len(path)]
                write_bytes(frame_path, bytes([pr, pc, pc, 0, 0, 0]))
                enable_custom(dev)

                time.sleep(frame_time)

        elif pattern == 'bar':
            # Vertical bar moving across columns (writes whole rows each frame).
            r1, g1, b1 = rgb
            r0, g0, b0 = rgb2
            for i in range(total_frames):
                cc = i % cols
                for rr in range(rows):
                    # Build a row: background rgb2, with one column in rgb.
                    row_bytes = bytearray()
                    for c in range(cols):
                        if c == cc:
                            row_bytes += bytes([r1, g1, b1])
                        else:
                            row_bytes += bytes([r0, g0, b0])
                    frame_write_row(dev, rr, start_col, stop_col, bytes(row_bytes))
                time.sleep(frame_time)

        elif pattern == 'row':
            # Each frame lights a whole row (alternating rgb and rgb2).
            for i in range(total_frames):
                rr = i % rows
                for rrr in range(rows):
                    use_on = rrr == rr
                    row_rgb = rgb if use_on else rgb2
                    frame_write_row(dev, rrr, start_col, stop_col, solid_rgb_bytes(cols, row_rgb))
                time.sleep(frame_time)

        elif pattern == 'col':
            # Each frame lights a whole column (alternating rgb and rgb2).
            r1, g1, b1 = rgb
            r0, g0, b0 = rgb2
            for i in range(total_frames):
                cc = i % cols
                for rr in range(rows):
                    # payload: ROW_ID, START_COL, STOP_COL, R, G, B (single pixel per row)
                    write_bytes(frame_path, bytes([rr, cc, cc, r1, g1, b1]))
                    enable_custom(dev)
                # clear previous column
                pc = (i - 1) % cols
                for rr in range(rows):
                    write_bytes(frame_path, bytes([rr, pc, pc, r0, g0, b0]))
                    enable_custom(dev)
                time.sleep(frame_time)

        else:
            raise SystemExit(f'Pattern not supported: {pattern}')

    except KeyboardInterrupt:
        print('\nInterrupted (Ctrl+C).')
    finally:
        if clear:
            clear_all()

    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description='Try OpenRazer keyboard effects by writing matrix_effect_* sysfs nodes (matches daemon payloads).'
    )
    parser.add_argument(
        '--device-glob',
        default='/sys/bus/hid/drivers/razerkbd/*',
        help='Device directory glob (default: /sys/bus/hid/drivers/razerkbd/*)'
    )

    sub = parser.add_subparsers(dest='cmd', required=True)

    sub.add_parser('list', help='List available matrix_* sysfs nodes')

    sub.add_parser('none', help='Turn off (matrix_effect_none)')
    sub.add_parser('spectrum', help='Spectrum cycling (matrix_effect_spectrum)')

    p_wave = sub.add_parser('wave', help='Wave effect (matrix_effect_wave)')
    p_wave.add_argument('--dir', type=int, default=1, help='Direction: 1 left->right, 2 right->left')

    p_static = sub.add_parser('static', help='Static color (matrix_effect_static)')
    p_static.add_argument('--rgb', type=str, default='0,255,0', help="RGB as 'R,G,B' (default: 0,255,0)")

    p_blink = sub.add_parser('blinking', help='Blinking color (matrix_effect_blinking)')
    p_blink.add_argument('--rgb', type=str, default='255,0,0', help="RGB as 'R,G,B' (default: 255,0,0)")

    p_reactive = sub.add_parser('reactive', help='Reactive effect (matrix_effect_reactive)')
    p_reactive.add_argument('--rgb', type=str, default='0,0,255', help="RGB as 'R,G,B' (default: 0,0,255)")
    p_reactive.add_argument('--speed', type=int, default=4, help='Speed: 1..4 (default: 4)')

    sub.add_parser('breath-random', help='Breath random (matrix_effect_breath)')

    p_breath_single = sub.add_parser('breath-single', help='Breath single (matrix_effect_breath)')
    p_breath_single.add_argument('--rgb', type=str, default='0,255,255', help="RGB as 'R,G,B' (default: 0,255,255)")

    p_breath_dual = sub.add_parser('breath-dual', help='Breath dual (matrix_effect_breath)')
    p_breath_dual.add_argument('--rgb', type=str, default='255,0,0', help="RGB1 as 'R,G,B' (default: 255,0,0)")
    p_breath_dual.add_argument('--rgb2', type=str, default='0,0,255', help="RGB2 as 'R,G,B' (default: 0,0,255)")

    p_breath_triple = sub.add_parser('breath-triple', help='Breath triple (matrix_effect_breath)')
    p_breath_triple.add_argument('--rgb', type=str, default='255,0,0', help="RGB1 as 'R,G,B' (default: 255,0,0)")
    p_breath_triple.add_argument('--rgb2', type=str, default='0,255,0', help="RGB2 as 'R,G,B' (default: 0,255,0)")
    p_breath_triple.add_argument('--rgb3', type=str, default='0,0,255', help="RGB3 as 'R,G,B' (default: 0,0,255)")

    p_star_rand = sub.add_parser('starlight-random', help='Starlight random (matrix_effect_starlight)')
    p_star_rand.add_argument('--speed', type=int, default=2, help='Speed byte (0..255). Typical values: 1..3')

    p_star_single = sub.add_parser('starlight-single', help='Starlight single (matrix_effect_starlight)')
    p_star_single.add_argument('--speed', type=int, default=2, help='Speed byte (0..255). Typical values: 1..3')
    p_star_single.add_argument('--rgb', type=str, default='255,255,255', help="RGB as 'R,G,B' (default: 255,255,255)")

    p_star_dual = sub.add_parser('starlight-dual', help='Starlight dual (matrix_effect_starlight)')
    p_star_dual.add_argument('--speed', type=int, default=2, help='Speed byte (0..255). Typical values: 1..3')
    p_star_dual.add_argument('--rgb', type=str, default='255,0,0', help="RGB1 as 'R,G,B' (default: 255,0,0)")
    p_star_dual.add_argument('--rgb2', type=str, default='0,0,255', help="RGB2 as 'R,G,B' (default: 0,0,255)")

    sub.add_parser('custom', help='Switch to custom mode (matrix_effect_custom=1)')

    p_fp = sub.add_parser('frame-pixel', help='Set one pixel via matrix_custom_frame (requires custom mode)')
    p_fp.add_argument('--row', type=int, required=True, help='Row index (0-based)')
    p_fp.add_argument('--col', type=int, required=True, help='Col index (0-based)')
    p_fp.add_argument('--rgb', type=str, default='0,0,255', help="RGB as 'R,G,B' (default: 0,0,255)")
    p_fp.add_argument('--hold', type=float, default=0.0, help='Seconds to wait after setting pixel (default: 0)')
    p_fp.add_argument('--clear', action='store_true', help='Clear the pixel after --hold')

    p_ff = sub.add_parser('frame-fill', help='Fill the matrix via matrix_custom_frame (requires custom mode)')
    p_ff.add_argument('--rows', type=int, default=6, help='Rows to fill (default: 6)')
    p_ff.add_argument('--cols', type=int, default=18, help='Cols to fill (default: 18)')
    p_ff.add_argument('--pattern', choices=['solid', 'checker'], default='solid', help='Fill pattern')
    p_ff.add_argument('--rgb', type=str, default='0,0,255', help="RGB1 as 'R,G,B' (default: 0,0,255)")
    p_ff.add_argument('--rgb2', type=str, default='255,0,0', help="RGB2 for checker as 'R,G,B' (default: 255,0,0)")
    p_ff.add_argument('--hold', type=float, default=3.0, help='Seconds to hold before clearing (default: 3.0)')
    p_ff.add_argument('--no-clear', action='store_true', help='Do not clear after --hold')

    p_fs = sub.add_parser('frame-scan', help='Interactively scan (row,col) visibility via matrix_custom_frame')
    p_fs.add_argument('--rows', type=int, default=6, help='Rows to test (default: 6)')
    p_fs.add_argument('--cols', type=int, default=18, help='Cols to test (default: 18)')
    p_fs.add_argument('--rgb', type=str, default='0,0,255', help="RGB as 'R,G,B' (default: 0,0,255)")

    p_fa = sub.add_parser('frame-animate', help='Animate patterns via matrix_custom_frame (tests refresh/throughput)')
    p_fa.add_argument('--rows', type=int, default=6, help='Rows (default: 6)')
    p_fa.add_argument('--cols', type=int, default=18, help='Cols (default: 18)')
    p_fa.add_argument('--pattern', choices=['snake', 'bar', 'row', 'col'], default='snake', help='Animation pattern')
    p_fa.add_argument('--rgb', type=str, default='0,0,255', help="Primary RGB as 'R,G,B' (default: 0,0,255)")
    p_fa.add_argument('--rgb2', type=str, default='0,0,0', help="Secondary RGB as 'R,G,B' (default: 0,0,0)")
    p_fa.add_argument('--fps', type=float, default=30.0, help='Frames per second (default: 30)')
    p_fa.add_argument('--seconds', type=float, default=5.0, help='Duration in seconds (default: 5)')
    p_fa.add_argument('--no-clear', action='store_true', help='Do not clear at the end (default clears)')

    args = parser.parse_args()

    dev_root = pick_device_dir(args.device_glob)
    dev = Device(dev_root)

    if args.cmd == 'list':
        return cmd_list(dev)

    if args.cmd == 'none':
        return cmd_none(dev)

    if args.cmd == 'spectrum':
        return cmd_spectrum(dev)

    if args.cmd == 'wave':
        return cmd_wave(dev, args.dir)

    if args.cmd == 'static':
        try:
            rgb = parse_rgb(args.rgb)
        except ValueError:
            print("--rgb must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_static(dev, rgb)

    if args.cmd == 'blinking':
        try:
            rgb = parse_rgb(args.rgb)
        except ValueError:
            print("--rgb must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_blinking(dev, rgb)

    if args.cmd == 'reactive':
        try:
            rgb = parse_rgb(args.rgb)
        except ValueError:
            print("--rgb must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_reactive(dev, rgb, args.speed)

    if args.cmd == 'breath-random':
        return cmd_breath_random(dev)

    if args.cmd == 'breath-single':
        try:
            rgb = parse_rgb(args.rgb)
        except ValueError:
            print("--rgb must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_breath_single(dev, rgb)

    if args.cmd == 'breath-dual':
        try:
            rgb1 = parse_rgb(args.rgb)
            rgb2 = parse_rgb(args.rgb2)
        except ValueError:
            print("--rgb/--rgb2 must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_breath_dual(dev, rgb1, rgb2)

    if args.cmd == 'breath-triple':
        try:
            rgb1 = parse_rgb(args.rgb)
            rgb2 = parse_rgb(args.rgb2)
            rgb3 = parse_rgb(args.rgb3)
        except ValueError:
            print("--rgb/--rgb2/--rgb3 must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_breath_triple(dev, rgb1, rgb2, rgb3)

    if args.cmd == 'starlight-random':
        return cmd_starlight_random(dev, args.speed)

    if args.cmd == 'starlight-single':
        try:
            rgb = parse_rgb(args.rgb)
        except ValueError:
            print("--rgb must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_starlight_single(dev, rgb, args.speed)

    if args.cmd == 'starlight-dual':
        try:
            rgb1 = parse_rgb(args.rgb)
            rgb2 = parse_rgb(args.rgb2)
        except ValueError:
            print("--rgb/--rgb2 must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2
        return cmd_starlight_dual(dev, rgb1, rgb2, args.speed)

    if args.cmd == 'custom':
        return cmd_custom(dev)

    if args.cmd == 'frame-pixel':
        try:
            rgb = parse_rgb(args.rgb)
        except ValueError:
            print("--rgb must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2

        row = parse_int('--row', args.row, min_value=0, max_value=255)
        col = parse_int('--col', args.col, min_value=0, max_value=255)
        hold = float(args.hold)
        return cmd_frame_pixel(dev, row, col, rgb, hold, bool(args.clear))

    if args.cmd == 'frame-fill':
        try:
            rgb = parse_rgb(args.rgb)
            rgb2 = parse_rgb(args.rgb2)
        except ValueError:
            print("--rgb/--rgb2 must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2

        rows = parse_int('--rows', args.rows, min_value=1, max_value=255)
        cols = parse_int('--cols', args.cols, min_value=1, max_value=255)
        hold = float(args.hold)
        return cmd_frame_fill(dev, rows, cols, args.pattern, rgb, rgb2, hold, clear=(not args.no_clear))

    if args.cmd == 'frame-scan':
        try:
            rgb = parse_rgb(args.rgb)
        except ValueError:
            print("--rgb must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2

        rows = parse_int('--rows', args.rows, min_value=1, max_value=255)
        cols = parse_int('--cols', args.cols, min_value=1, max_value=255)
        return cmd_frame_scan(dev, rows, cols, rgb)

    if args.cmd == 'frame-animate':
        try:
            rgb = parse_rgb(args.rgb)
            rgb2 = parse_rgb(args.rgb2)
        except ValueError:
            print("--rgb/--rgb2 must be 'R,G,B' with values 0..255", file=sys.stderr)
            return 2

        rows = parse_int('--rows', args.rows, min_value=1, max_value=255)
        cols = parse_int('--cols', args.cols, min_value=1, max_value=255)
        fps = float(args.fps)
        seconds = float(args.seconds)
        return cmd_frame_animate(
            dev,
            rows,
            cols,
            args.pattern,
            rgb,
            rgb2,
            fps,
            seconds,
            clear=(not args.no_clear),
        )

    raise SystemExit(f"Unhandled cmd: {args.cmd}")


if __name__ == '__main__':
    raise SystemExit(main())
