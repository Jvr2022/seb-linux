<div align="center">
  <img src="assets/icons/safe-exam-browser.png" alt="Safe Exam Browser for Linux logo" width="120" />

  # Safe Exam Browser for Linux

  Hey there! This is a community-driven, native Linux version of 
  Safe Exam Browser. We built it with Qt 6 to give Linux users
  a reliable way to take exams without needing a specific OS.
</div>

## What is this?

We're trying to make SEB work great on Linux. This client handles all
the important stuff like `.seb` files and special links (`seb://`),
so you can just focus on your exam.

### Running Everywhere

Safe Exam Browser has options for QtWebEngine and webkitGTK.
We've also packaged all of the dependencies inside AppImage files so
Safe Exam Browser for Linux really can run anywhere.

## Getting Started

### Prerequisites

> [!NOTE]  
> RISC-V or ARM AppImages are not (yet) available.

You'll need the usual Qt 6 development tools. If you're on something like RISC-V
where Qt WebEngine isn't around, make sure you've got `libwebkit2gtk-4.1-dev`
and `libgtk-3-dev` installed so we can use the fallback engine.

### How to Build

It's pretty simple:

```bash
./scripts/build.sh
```

You'll find the binary at `build/bin/safe-exam-browser`.

### Running an Exam

Just point it at your `.seb` file:

```bash
./build/bin/safe-exam-browser /path/to/my-exam.seb
```

Or use a link:

```bash
./build/bin/safe-exam-browser sebs://exam-link.seb
```

## Troubleshooting & Support

If you run the app and see a message saying
**"Safe Exam Browser is not supported on your device"**,
it means we couldn't find a compatible browser engine on your system.

We want to fix that! 
please [open an issue here](https://github.com/Jvr2022/seb-linux/issues)
with some details about your system and what version of Linux you're using.

## License

This is an open-source project licensed under the `MPL2`.
Check the `LICENSE` file for the legalese.
