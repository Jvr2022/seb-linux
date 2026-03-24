<div align="center">
  <img src="assets/icons/safe-exam-browser.png" alt="Safe Exam Browser for Linux logo" width="120" />

  # Safe Exam Browser for Linux

  **A native Linux implementation of Safe Exam Browser, built with Qt 6 and Qt WebEngine.**
</div>

## Overview

This project provides a native Linux client focused on:

- Reliable startup and kiosk-like exam runtime behavior
- Linux desktop integration (file associations and protocol handling)
- Packaging and distribution for common Linux ecosystems

It supports:

- Opening `.seb` configuration files
- Handling `seb://` and `sebs://` links
- Running as an installed Linux desktop application

> [!IMPORTANT]
> This repository is an independent Linux implementation and is **not** an official Safe Exam Browser release.

## Prerequisites

Make sure your environment has the dependencies required for a Qt 6 + Qt WebEngine desktop build.

## Build

```bash
./scripts/build.sh
```

Compiled binary output:

```bash
build/bin/safe-exam-browser
```

## Run

Start with default behavior:

```bash
./build/bin/safe-exam-browser
```

Open a local exam file:

```bash
./build/bin/safe-exam-browser /path/to/exam.seb
```

Open a remote exam link:

```bash
./build/bin/safe-exam-browser sebs://demo.safeexambrowser.org/exams/DemoExamGeneral.seb
```

Run with a JSON config file:

```bash
./build/bin/safe-exam-browser --config ./examples/minimal-config.json
```

## Create Release Artifacts

```bash
./scripts/build-release.sh 0.1.0
```

Release files are written to `dist/`.

Prebuilt binaries are available from the GitHub Releases page.

## Installation

### Debian/Ubuntu

```bash
sudo apt install ./dist/safe-exam-browser_0.1.0_amd64.deb
```

### Arch Linux

```bash
cd packaging/arch
makepkg -si
```

## Contributing

Please review these files before opening a PR:

- [CONTRIBUTING.md](./CONTRIBUTING.md)
- [CODE_OF_CONDUCT.md](./CODE_OF_CONDUCT.md)
- [SECURITY.md](./SECURITY.md)

Quick copy/open helper:

```bash
cat CONTRIBUTING.md CODE_OF_CONDUCT.md SECURITY.md
```

## License

Licensed under the terms in `LICENSE`.
