"""Smoke tests for the nes_py bindings (src/python/Bindings.cpp).

The emulator itself is covered by the C++ tests; these check the binding
layer: shapes and dtypes, copies vs views, error mapping, save/load
determinism, and .nesdemo recordings (including replay determinism). They
run against the committed nestest ROM, and also against the ROM in $NES_ROM
when it is set.
"""

import os
import struct
from pathlib import Path

import numpy as np
import pytest

import nes_py

NESTEST_ROM = Path(__file__).resolve().parents[1] / "roms" / "nestest.nes"

# Save state header: magic(4) version(2) mapper(1) prg_size(4) chr_size(4) checksum(4) payload_size(4)
HEADER_SIZE = 23
CHECKSUM_OFFSET = 15

# Recording header: magic(4) version(2) rom_checksum(4), then state_size(4) + state, frame_count(4) + buttons
RECORDING_VERSION_OFFSET = 4
RECORDING_CHECKSUM_OFFSET = 6

ROMS = [pytest.param(NESTEST_ROM, id="nestest")]
if os.environ.get("NES_ROM"):
    ROMS.append(pytest.param(Path(os.environ["NES_ROM"]), id="NES_ROM"))


@pytest.fixture(params=ROMS)
def rom_path(request):
    return str(request.param)


@pytest.fixture
def core(rom_path):
    return nes_py.NesCore(rom_path)


def play(core):
    """A fixed input sequence used by the determinism tests."""
    core.step(0, frames=10)
    core.step(nes_py.BUTTON_START, frames=5)
    core.step(nes_py.BUTTON_RIGHT | nes_py.BUTTON_A, frames=30)
    core.step(0, frames=15)


# The same inputs as play(), one mask per frame
PLAY_BUTTONS = np.array(
    [0] * 10 + [nes_py.BUTTON_START] * 5 + [nes_py.BUTTON_RIGHT | nes_py.BUTTON_A] * 30 + [0] * 15,
    dtype=np.uint8,
)


def record(core, buttons):
    """Play buttons one frame at a time from the current state and return the Recording."""
    start_state = core.save_state()
    for mask in buttons:
        core.step(int(mask))
    return nes_py.Recording(core.rom_checksum(), start_state, buttons)


def reference_obs84(frame):
    """numpy version of nes::ai::ToObservation: integer BT.601 luma, then integer-box averaging."""
    rgb = frame.astype(np.uint32)
    luma = (77 * rgb[..., 0] + 150 * rgb[..., 1] + 29 * rgb[..., 2]) >> 8
    height, width = luma.shape
    size = nes_py.OBS_SIZE
    obs = np.empty((size, size), dtype=np.uint8)
    for oy in range(size):
        y0, y1 = oy * height // size, (oy + 1) * height // size
        for ox in range(size):
            x0, x1 = ox * width // size, (ox + 1) * width // size
            box = luma[y0:y1, x0:x1]
            obs[oy, ox] = (int(box.sum()) + box.size // 2) // box.size
    return obs


# --- Module surface ---


def test_module_constants():
    assert nes_py.OBS_SIZE == 84
    buttons = [
        nes_py.BUTTON_A,
        nes_py.BUTTON_B,
        nes_py.BUTTON_SELECT,
        nes_py.BUTTON_START,
        nes_py.BUTTON_UP,
        nes_py.BUTTON_DOWN,
        nes_py.BUTTON_LEFT,
        nes_py.BUTTON_RIGHT,
    ]
    assert buttons == [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]


def test_missing_rom_raises_runtime_error(tmp_path):
    with pytest.raises(RuntimeError):
        nes_py.NesCore(str(tmp_path / "missing.nes"))


# --- Array outputs ---


@pytest.mark.parametrize(
    ("method", "shape"),
    [("frame", (240, 256, 3)), ("obs84", (84, 84)), ("ram", (2048,))],
)
def test_array_shape_and_dtype(core, method, shape):
    core.step(0, frames=5)
    array = getattr(core, method)()
    assert array.shape == shape
    assert array.dtype == np.uint8
    assert array.flags.c_contiguous


@pytest.mark.parametrize("method", ["frame", "obs84", "ram"])
def test_arrays_are_copies_not_views(core, method):
    core.step(0, frames=5)
    before = getattr(core, method)()
    snapshot = before.copy()

    play(core)
    np.testing.assert_array_equal(before, snapshot)  # unchanged by stepping

    current = getattr(core, method)()
    current[...] = 0xAA
    assert not np.array_equal(getattr(core, method)(), current)  # writing doesn't reach the emulator


def test_obs84_matches_frame(core):
    play(core)
    np.testing.assert_array_equal(core.obs84(), reference_obs84(core.frame()))


# --- step() ---


def test_step_rejects_zero_frames(core):
    with pytest.raises(ValueError):
        core.step(0, frames=0)


def test_step_frames_is_keyword_only(core):
    with pytest.raises(TypeError):
        core.step(nes_py.BUTTON_RIGHT, 4)


def test_step_rejects_buttons_outside_uint8(core):
    with pytest.raises(TypeError):
        core.step(256)


def test_step_advances_the_emulator(core):
    before_frame, before_ram = core.frame(), core.ram()
    play(core)
    assert not np.array_equal(before_frame, core.frame()) or not np.array_equal(before_ram, core.ram())


# --- Save states ---


def test_save_load_is_deterministic(core):
    core.step(0, frames=5)
    state = core.save_state()

    play(core)
    first_ram, first_frame = core.ram(), core.frame()

    core.load_state(state)
    play(core)
    np.testing.assert_array_equal(core.ram(), first_ram)
    np.testing.assert_array_equal(core.frame(), first_frame)


def test_save_state_uses_the_file_format(core):
    state = core.save_state()
    assert isinstance(state, bytes)
    assert state[:4] == b"NESS"
    (payload_size,) = struct.unpack_from("<I", state, HEADER_SIZE - 4)
    assert payload_size == len(state) - HEADER_SIZE


def test_load_state_rejects_junk(core):
    with pytest.raises(RuntimeError):
        core.load_state(b"junk")


def test_load_state_rejects_truncated_state(core):
    state = core.save_state()
    with pytest.raises(RuntimeError):
        core.load_state(state[:-1])


def test_load_state_rejects_state_from_another_rom(core):
    # Altering the checksum simulates a state saved with a different game loaded
    state = bytearray(core.save_state())
    state[CHECKSUM_OFFSET] ^= 0xFF
    with pytest.raises(RuntimeError):
        core.load_state(bytes(state))


def test_failed_load_leaves_state_unchanged(core):
    play(core)
    before = core.ram()
    with pytest.raises(RuntimeError):
        core.load_state(core.save_state()[:-1])
    np.testing.assert_array_equal(core.ram(), before)


# --- rom_checksum() ---


def test_rom_checksum_is_stable_and_matches_save_state_header(core, rom_path):
    checksum = core.rom_checksum()
    assert checksum == nes_py.NesCore(rom_path).rom_checksum()
    (header_checksum,) = struct.unpack_from("<I", core.save_state(), CHECKSUM_OFFSET)
    assert header_checksum == checksum


# --- Recordings ---


def test_recording_properties(core):
    start_state = core.save_state()
    recording = nes_py.Recording(core.rom_checksum(), start_state, PLAY_BUTTONS)

    assert len(recording) == len(PLAY_BUTTONS)
    assert recording.rom_checksum == core.rom_checksum()
    assert isinstance(recording.start_state, bytes)
    assert recording.start_state == start_state
    buttons = recording.buttons
    assert buttons.shape == (len(PLAY_BUTTONS),)
    assert buttons.dtype == np.uint8
    assert buttons.flags.c_contiguous
    np.testing.assert_array_equal(buttons, PLAY_BUTTONS)


def test_recording_with_no_frames():
    recording = nes_py.Recording(0, b"", np.array([], dtype=np.uint8))
    assert len(recording) == 0
    assert recording.buttons.shape == (0,)
    assert recording.start_state == b""


def test_recording_buttons_are_copies():
    source = np.array([1, 2, 3], dtype=np.uint8)
    recording = nes_py.Recording(0, b"", source)

    source[:] = 0  # changing the input after construction doesn't reach the recording
    first = recording.buttons
    first[:] = 0xAA  # writing to a returned array doesn't either
    np.testing.assert_array_equal(recording.buttons, [1, 2, 3])
    assert recording.buttons is not recording.buttons


@pytest.mark.parametrize(
    "buttons",
    [
        pytest.param(np.array([1, 2], dtype=np.int64), id="int64"),
        pytest.param(np.zeros((2, 2), dtype=np.uint8), id="2-D"),
        pytest.param([1, 2], id="list"),
        pytest.param(np.zeros(8, dtype=np.uint8)[::2], id="strided"),
    ],
)
def test_recording_rejects_buttons_that_are_not_a_contiguous_1d_uint8_array(buttons):
    with pytest.raises(TypeError):
        nes_py.Recording(0, b"", buttons)


def test_save_and_load_recording_round_trip(core, tmp_path):
    original = record(core, PLAY_BUTTONS)
    path = tmp_path / "clip.nesdemo"

    nes_py.save_recording(original, str(path))
    loaded = nes_py.load_recording(str(path))

    assert loaded.rom_checksum == original.rom_checksum
    assert loaded.start_state == original.start_state
    np.testing.assert_array_equal(loaded.buttons, original.buttons)
    assert [p.name for p in tmp_path.iterdir()] == ["clip.nesdemo"]  # no temp file left behind


def test_save_recording_uses_the_file_format(core, tmp_path):
    path = tmp_path / "clip.nesdemo"
    nes_py.save_recording(record(core, PLAY_BUTTONS), str(path))

    data = path.read_bytes()
    assert data[:4] == b"NESR"
    assert struct.unpack_from("<H", data, RECORDING_VERSION_OFFSET) == (1,)
    assert struct.unpack_from("<I", data, RECORDING_CHECKSUM_OFFSET) == (core.rom_checksum(),)
    assert data[-len(PLAY_BUTTONS):] == PLAY_BUTTONS.tobytes()


def test_save_recording_to_a_missing_directory_raises_runtime_error(core, tmp_path):
    with pytest.raises(RuntimeError):
        nes_py.save_recording(record(core, PLAY_BUTTONS), str(tmp_path / "missing" / "clip.nesdemo"))


def test_replaying_a_recording_reproduces_the_session(core, rom_path, tmp_path):
    core.step(0, frames=5)
    recording = record(core, PLAY_BUTTONS)
    final_ram, final_frame = core.ram(), core.frame()
    path = tmp_path / "clip.nesdemo"
    nes_py.save_recording(recording, str(path))

    # Replay in a fresh emulator, one frame at a time, as the training data conversion will
    replay = nes_py.NesCore(rom_path)
    loaded = nes_py.load_recording(str(path))
    assert loaded.rom_checksum == replay.rom_checksum()
    replay.load_state(loaded.start_state)
    for mask in loaded.buttons:
        replay.step(int(mask))

    np.testing.assert_array_equal(replay.ram(), final_ram)
    np.testing.assert_array_equal(replay.frame(), final_frame)


def test_load_recording_missing_file_raises_runtime_error(tmp_path):
    with pytest.raises(RuntimeError):
        nes_py.load_recording(str(tmp_path / "missing.nesdemo"))


@pytest.mark.parametrize(
    "corrupt",
    [
        pytest.param(lambda data: b"junk", id="junk"),
        pytest.param(lambda data: b"", id="empty"),
        pytest.param(lambda data: data[:5], id="truncated-header"),
        pytest.param(lambda data: data[:-1], id="missing-button"),
        pytest.param(lambda data: data + b"\x00", id="extra-byte"),
        pytest.param(lambda data: data[:4] + b"\x02\x00" + data[6:], id="unsupported-version"),
    ],
)
def test_load_recording_rejects_corrupt_files(core, tmp_path, corrupt):
    path = tmp_path / "clip.nesdemo"
    nes_py.save_recording(record(core, PLAY_BUTTONS), str(path))
    path.write_bytes(corrupt(path.read_bytes()))

    with pytest.raises(RuntimeError):
        nes_py.load_recording(str(path))
