# Safe Exam Browser for Linux

Safe Exam Browser for Linux is a Qt 6 and Qt WebEngine based Linux desktop implementation focused on native startup, Linux desktop integration, and release packaging.

It supports opening SEB configuration files, handling `seb://` and `sebs://` links, and running as an installed Linux desktop application.

## Build

```bash
./scripts/build.sh
```

The compiled binary is written to:

```bash
build/bin/safe-exam-browser
```

## Run

```bash
./build/bin/safe-exam-browser
./build/bin/safe-exam-browser /path/to/exam.seb
./build/bin/safe-exam-browser sebs://demo.safeexambrowser.org/exams/DemoExamGeneral.seb
./build/bin/safe-exam-browser --config ./examples/minimal-config.json
```

## Release Artifacts

```bash
./scripts/build-release.sh 0.1.0
```

Artifacts are written to `dist/`.

## Install

Debian-based systems:

```bash
sudo apt install ./dist/safe-exam-browser_0.1.0_amd64.deb
```

Arch Linux:

```bash
cd packaging/arch
makepkg -si
```

## Notice

This repository is an independent Linux implementation and is not an official Safe Exam Browser release.
