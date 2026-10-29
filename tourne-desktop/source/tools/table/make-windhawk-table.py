"""Tourne'Table for the Windhawk catalog, made from Tourne Desktop's built-in Table (tourne-table.wh.cpp).

Everything from 2.2 and 2.3 stays (Engine Cores included), except the frame pacing additions: Frame Sync, Low
Latency and Frame Generation. Those go, and 2.1.1's Smooth Mode (Auto / On / Off) and its pacing code come back, as
the catalog has them. Run again whenever tourne-table.wh.cpp changes:

    python tools/table/make-windhawk-table.py
"""
import pathlib
import sys

root = pathlib.Path(__file__).resolve().parents[2]
src = (root / "tourne-table.wh.cpp").read_text(encoding="utf-8")
out = root / "tourne-table-windhawk.wh.cpp"
text = src


def replace(old, new, count=1):
    global text
    found = text.count(old)
    if found != count:
        sys.exit(f"expected {count} of:\n{old[:200]}\nfound {found}")
    text = text.replace(old, new)


def cut(start, end, new=""):
    """Replaces everything from start up to (not including) end, each found once."""
    global text
    a = text.find(start)
    if a < 0 or text.find(start, a + 1) >= 0:
        sys.exit(f"start not found once:\n{start[:200]}")
    b = text.find(end, a + len(start))
    if b < 0:
        sys.exit(f"end not found after start:\n{end[:200]}")
    text = text[:a] + new + text[b:]


# ---- Header: the catalog's id and name -------------------------------------------
replace("// @id                  tourne-table-v2\n", "// @id                  tourne-table-desktop-audio-visualizer\n")
replace("// @name                Tourne'Table [Audio Visualizer] (beta)\n",
        "// @name                Tourne'Table [Audio Visualizer]\n")

# ---- Readme ------------------------------------------------------------------------
replace("including the batching it does with Frame Sync on. Kept for comparison",
        "including Smooth Mode's batching. Kept for comparison")
cut("**Frame Sync** (called Smooth Mode up to 2.1)", "- **Frame timing locked to the display.**",
    "**Smooth Mode**: Frame timing locked to the display, and motion on elapsed time. **Auto** *(default)* turns it on "
    "when drawing runs on an integrated GPU or the CPU; **On** works on any GPU and is worth trying on a discrete one "
    "too; **Off** is the 1.4 timing. What it changes:\n\n")
replace("and never accumulates drift.\n- **Target FPS", "and never accumulates drift.\n- **Target FPS")
replace("Frame Sync schedules from Windows' own composition clock", "Smooth Mode schedules from Windows' own composition clock")
replace("Frame Sync runs the nearest whole fraction instead: 60 becomes 72 on 144 Hz, 48 if you set it to 50, 60 on a 60 Hz "
        "screen. With VSync, asking for more than the refresh rate gets the refresh rate; Adaptive and Fast Sync keep it.",
        "Smooth Mode runs the nearest whole fraction instead: 60 becomes 72 on 144 Hz, 48 if you set it to 50, 60 on a 60 Hz "
        "screen. Asking for more than the refresh rate gets the refresh rate rather than frames nobody can see.")

# ---- Settings schema ---------------------------------------------------------------
replace("Direct2D is the 1.5 path, unchanged, including the batching it does with Frame Sync on; it is kept",
        "Direct2D is the 1.5 path, unchanged, including Smooth Mode's batching; it is kept")
cut("    - frameSync: auto\n", "    - engineCores: auto\n",
    "    - smoothMode: auto\n"
    "      $name: Smooth Mode\n"
    "      $description: Frame timing locked to the display's refresh so every frame lands on its own refresh, and motion "
    "that scales with real elapsed time. Target FPS is rounded to an even fraction of the refresh rate (60 on a 144 Hz "
    "screen runs at 72) because that is what makes motion even. With the Direct2D renderer it also bakes the panel and "
    "batches the bars. Auto turns it on when drawing runs on an integrated GPU or the CPU. On works with any GPU. Off is "
    "the 1.4 timing\n"
    "      $options:\n"
    "        - auto: Auto (integrated GPU or CPU drawing)\n"
    "        - 'on': 'On'\n"
    "        - 'off': Off (1.4 timing)\n")

# ---- Settings: Smooth Mode in place of Frame Sync and the rest ------------------------
replace("enum class VizFrameSync { Auto, Off, VSync, Adaptive, Fast };\n"
        "enum class VizLowLatency { Off, On, Ultra };\n",
        "enum class VizSmoothMode { Auto, On, Off };\n")
replace("    VizFrameSync frameSync = VizFrameSync::Auto;\n"
        "    VizLowLatency lowLatency = VizLowLatency::Off;\n"
        "    bool frameGenX2 = false;\n",
        "    VizSmoothMode smoothMode = VizSmoothMode::Auto;\n")
replace("    VizFrameSync sync = g_settings.frameSync;\n"
        "    bool on = sync == VizFrameSync::VSync || sync == VizFrameSync::Adaptive || sync == VizFrameSync::Fast ||\n"
        "              (sync == VizFrameSync::Auto && g_renderAdapter.valid &&\n",
        "    bool on = g_settings.smoothMode == VizSmoothMode::On ||\n"
        "              (g_settings.smoothMode == VizSmoothMode::Auto && g_renderAdapter.valid &&\n")
cut("    PCWSTR frameSync = VizSettingString(L\"hardware.frameSync\");\n",
    "    PCWSTR cores = VizSettingString(L\"hardware.engineCores\");\n",
    "    PCWSTR smoothMode = VizSettingString(L\"hardware.smoothMode\");\n"
    "    g_settings.smoothMode = (wcscmp(smoothMode, L\"on\") == 0)    ? VizSmoothMode::On\n"
    "                          : (wcscmp(smoothMode, L\"off\") == 0)   ? VizSmoothMode::Off\n"
    "                                                                : VizSmoothMode::Auto;\n"
    "    VizSettingFree(smoothMode);\n\n")
replace("    {L\"hardware.frameSync\", L\"auto\", true, 2},\n"
        "    {L\"hardware.lowLatency\", L\"off\", true, 2},\n"
        "    {L\"hardware.frameGen\", L\"off\", true, 2},\n",
        "    {L\"hardware.smoothMode\", L\"auto\", true, 2},\n")
cut("    {L\"hardware.frameSync\", L\"Hardware\", L\"\", L\"Frame Sync\", 3,", "    {L\"hardware.engineCores\", L\"Hardware\",",
    "    {L\"hardware.smoothMode\", L\"Hardware\", L\"\", L\"Smooth Mode\", 3, L\"auto\", "
    "L\"auto=Auto (integrated GPU or CPU drawing)\\non=On\\noff=Off (1.4 timing)\",\n"
    "     L\"Locks frames to your screen's refresh and moves things by real time.\\nAuto: on with integrated graphics or "
    "CPU drawing.\\nOn: on with any GPU; smoothest.\\nOff: the 1.4 timing.\\nTry: On for the smoothest motion.\"},\n")

# ---- Frame Generation ---------------------------------------------------------------
cut("void PublishBandFrameNow(const VizBandFrame& f);\n\n// Frame Generation x2.", "void PublishBandFrameNow(const VizBandFrame& f) {")
text = text.replace("PublishBandFrameNow(", "PublishBandFrame(")
replace("        // Frame Generation x2: every other tick shows the frame the last\n"
        "        // analysis held back instead of analysing.\n"
        "        if (!(g_settings.frameGenX2 && (fgTick++ & 1) && VizFrameGenShowHeld())) {\n"
        "            g_fgInterp = g_settings.frameGenX2;\n"
        "            engineFrame();\n"
        "            g_fgInterp = false;\n"
        "        }\n",
        "        engineFrame();\n")

# ---- Low Latency, Frame Sync's pacing: 2.1.1's Smooth Mode pacing back -----------------
cut("// The slowest recent frame's drawing time in ms, forgotten slowly (2% a\n", "struct VizVsyncSchedule {\n")
replace("    static LONGLONG Next(LONGLONG due, LONGLONG now, LONGLONG vblank, LONGLONG period, int divisor, double phase) {\n"
        "        LONGLONG next = AlignUp(due + (LONGLONG)divisor * period - period / 2, vblank, period, phase);\n"
        "        if (next <= now) next = AlignUp(now + 1, vblank, period, phase);\n",
        "    static LONGLONG Next(LONGLONG due, LONGLONG now, LONGLONG vblank, LONGLONG period, int divisor) {\n"
        "        LONGLONG next = AlignUp(due + (LONGLONG)divisor * period - period / 2, vblank, period, 0.5);\n"
        "        if (next <= now) next = AlignUp(now + 1, vblank, period, 0.5);\n")
cut("            // Where in the refresh each frame starts (Low Latency): half a\n",
    "            } else {\n                LONGLONG interval",
    "            if (vsPeriod > 0) {\n"
    "                if (vsDue == 0) vsDue = VizVsyncSchedule::AlignUp(nowQpc.QuadPart, vsVblank, vsPeriod, 0.5);\n"
    "                if (nowQpc.QuadPart < vsDue) {\n"
    "                    preciseWait((double)(vsDue - nowQpc.QuadPart) * 1000.0 / (double)qpcFreq.QuadPart);\n"
    "                    continue;\n"
    "                }\n"
    "                int divisor = VizVsyncSchedule::Divisor((double)fps, vsPeriod, qpcFreq.QuadPart);\n"
    "                vsDue = VizVsyncSchedule::Next(vsDue, nowQpc.QuadPart, vsVblank, vsPeriod, divisor);\n"
    "                absDue = 0;\n")

# ---- Low Latency's draw-time measuring (Engine Cores stays) ----------------------------
replace("            g_renderTickPending.store(false, std::memory_order_relaxed);\n"
        "            if (!g_unloading && !g_fullscreenPaused.load()) {\n"
        "                LARGE_INTEGER t0, t1, fq;\n"
        "                QueryPerformanceCounter(&t0);\n"
        "                RenderVisualizer();\n"
        "                QueryPerformanceCounter(&t1);\n"
        "                QueryPerformanceFrequency(&fq);\n"
        "                VizNoteDrawMs((float)((double)(t1.QuadPart - t0.QuadPart) * 1000.0 / (double)fq.QuadPart));\n"
        "            }\n",
        "            g_renderTickPending.store(false, std::memory_order_relaxed);\n"
        "            if (!g_unloading && !g_fullscreenPaused.load()) {\n"
        "                RenderVisualizer();\n"
        "            }\n")
replace("    unsigned fgTick = 0;\n", "")

# ---- Nothing of the three left --------------------------------------------------------
left = [w for w in ("FrameSync", "frameSync", "LowLatency", "lowLatency", "frameGen", "FrameGen", "g_fg",
                    "g_drawMsMax", "VizNoteDrawMs", "Frame Sync", "Low Latency", "Frame Generation")
        if w in text]
if left:
    sys.exit("still in the file: " + ", ".join(left))
out.write_text(text, encoding="utf-8", newline="\r\n")  # (as the source)
print(f"wrote {out} ({len(text)} chars)")

# A copy in Downloads (wherever Windows keeps it), where Tourne installs Windhawk mods from.
if sys.platform == "win32":
    import ctypes, uuid
    from ctypes import wintypes
    folder = ctypes.c_wchar_p()
    downloads = uuid.UUID("374DE290-123F-4565-9164-39C4925E467B")  # FOLDERID_Downloads
    guid = (ctypes.c_byte * 16).from_buffer_copy(downloads.bytes_le)
    if ctypes.windll.shell32.SHGetKnownFolderPath(ctypes.byref(guid), 0, None, ctypes.byref(folder)) == 0:
        copy = pathlib.Path(folder.value) / "tourne-table-2.3.0-windhawk.wh.cpp"
        ctypes.windll.ole32.CoTaskMemFree(folder)
        copy.write_text(text, encoding="utf-8", newline="\r\n")
        print(f"and {copy}")
