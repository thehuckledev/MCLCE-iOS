#!/usr/bin/env python3
"""bracket ColourTable::getColour to see what id and this are at the crash"""
import sys
from pathlib import Path
TARGET = Path(__file__).resolve().parent.parent / "upstream" / "Minecraft.Client" / "Common" / "Colours" / "ColourTable.cpp"
src = TARGET.read_text(encoding="utf-8", errors="replace")
if "CT_CKPT" in src:
    print("already patched"); sys.exit(0)

old = (
    "unsigned int ColourTable::getColour(eMinecraftColour id)\n"
    "{\n"
    "\tint idx = static_cast<int>(id);\n"
    "\tif (idx >= 0 && idx < eMinecraftColour_COUNT)\n"
    "\t{\n"
    "\t\treturn m_colourValues[idx];\n"
    "\t}\n"
    "\treturn 0; // Return black for invalid IDs\n"
    "}"
)
new = (
    "unsigned int ColourTable::getColour(eMinecraftColour id)\n"
    "{\n"
    "\tstatic int s_logCount = 0;\n"
    "\tif (s_logCount < 5) {\n"
    "\t\tapp.DebugPrintf(\"CT_CKPT getColour this=%p id=%d\", this, (int)id);\n"
    "\t\ts_logCount++;\n"
    "\t}\n"
    "\tint idx = static_cast<int>(id);\n"
    "\tif (idx >= 0 && idx < eMinecraftColour_COUNT)\n"
    "\t{\n"
    "\t\treturn m_colourValues[idx];\n"
    "\t}\n"
    "\treturn 0; // Return black for invalid IDs\n"
    "}"
)
if old not in src: sys.exit("anchor not found")
src = src.replace(old, new, 1)
TARGET.write_text(src, encoding="utf-8")
print(f"patched {TARGET}")
