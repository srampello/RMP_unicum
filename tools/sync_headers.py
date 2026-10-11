#!/usr/bin/env python3
"""Mantener cada sketch Arduino independiente sin divergir el pinout."""
from pathlib import Path
import shutil
root=Path(__file__).resolve().parents[1]
source=root/'firmware/UNiCUM/RMP_UNICUM'
for sketch in (root/'firmware/pruebas').iterdir():
    if sketch.is_dir():
        for name in ('Config.h','Hardware.h'):
            shutil.copyfile(source/name, sketch/name)
