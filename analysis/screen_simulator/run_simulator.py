#!/usr/bin/env python3
from __future__ import annotations

import argparse
import pathlib
import shutil
import subprocess
import sys


SIM_DIR = pathlib.Path(__file__).resolve().parent
ECHO_PET_DIR = SIM_DIR.parents[1]
SOURCE_DIR = ECHO_PET_DIR / "src" if (ECHO_PET_DIR / "src").exists() else ECHO_PET_DIR
INCLUDE_DIR = ECHO_PET_DIR / "include"
BUILD_DIR = SIM_DIR / "build"
DEFAULT_OUT_DIR = SIM_DIR / "out"


def find_gxx() -> str:
    for candidate in (
        shutil.which("g++"),
        r"C:\MinGW\bin\g++.exe",
    ):
        if candidate and pathlib.Path(candidate).exists():
            return candidate
    raise SystemExit("g++ was not found. Install MinGW g++ or add it to PATH.")


def compile_simulator(gxx: str) -> pathlib.Path:
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    exe = BUILD_DIR / "echopet_screen_simulator.exe"
    sources = [
        SIM_DIR / "host_screen_simulator.cpp",
        SOURCE_DIR / "EchoPetDisplay.cpp",
        SOURCE_DIR / "EchoPetMenuIconResources.cpp",
        SOURCE_DIR / "EchoPetCatalogVisuals.cpp",
        SOURCE_DIR / "EchoPetCharacterVisuals.cpp",
        SOURCE_DIR / "EchoPetSprites.cpp",
        SOURCE_DIR / "EchoPetResources128.cpp",
        SOURCE_DIR / "EchoPetResources64.cpp",
        SOURCE_DIR / "EchoPetUi.cpp",
        SOURCE_DIR / "EchoPetCatalog.cpp",
        SOURCE_DIR / "EchoPetCharacterCatalog.cpp",
        SOURCE_DIR / "EchoPetModel.cpp",
    ]
    cmd = [
        gxx,
        "-std=c++17",
        "-O2",
        "-DHOST_SCREEN_SIMULATOR",
        "-I",
        str(SIM_DIR),
        "-I",
        str(SOURCE_DIR),
        "-I",
        str(INCLUDE_DIR),
        "-I",
        str(ECHO_PET_DIR),
        *map(str, sources),
        "-o",
        str(exe),
    ]
    print("compiling host simulator...")
    subprocess.run(cmd, cwd=ECHO_PET_DIR, check=True)
    return exe


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build and run the EchoPet host screen simulator."
    )
    parser.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT_DIR)
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument("--profile", choices=("all", "large", "compact"), default="all")
    parser.add_argument("--scenario", action="append", default=[])
    parser.add_argument("--phase", type=int)
    parser.add_argument("--build-only", action="store_true")
    parser.add_argument("--list", action="store_true")
    args = parser.parse_args()

    exe = compile_simulator(find_gxx())
    if args.build_only:
        print(f"built {exe}")
        return 0

    args.out.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(exe),
        "--out",
        str(args.out),
        "--scale",
        str(args.scale),
        "--profile",
        args.profile,
    ]
    if args.list:
        cmd.append("--list")
    if args.phase is not None:
        cmd.extend(["--phase", str(args.phase)])
    for scenario in args.scenario:
        cmd.extend(["--scenario", scenario])

    print("rendering screen PNGs...")
    subprocess.run(cmd, cwd=ECHO_PET_DIR, check=True)
    if not args.list:
        images = sorted(args.out.glob("*.png"))
        print(f"output: {args.out}")
        print(f"png files: {len(images)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

