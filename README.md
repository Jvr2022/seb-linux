<div align="center">
  <img src="assets/icons/safe-exam-browser.png" alt="Safe Exam Browser for Linux logo" width="120" />
</div>

# Safe Exam Browser for Linux

Hey there! This is a community-driven, native Linux version of
Safe Exam Browser. We built it with Qt 6 to give Linux users
a reliable way to take exams without needing a specific OS.

## What is this?

> [!NOTE]  
> Opening v3 sebs:// urls is now supported!
> This means password login exams will work again!

We're trying to make SEB work great on Linux. This client handles all
the important stuff like `.seb` files and special links (`seb://`, `sebs://`),
so you can just focus on your exam.

### Running Everywhere

Safe Exam Browser has options for QtWebEngine and WebKitGTK.
We've also packaged all of the dependencies inside AppImage files so
Safe Exam Browser for Linux really can run anywhere.

## Getting Started

### Prerequisites

> [!NOTE]  
> RISC-V or ARM AppImages are not (yet) available.
> Feel free to build them yourself or contribute to the
> building workflow.

You'll need the usual Qt 6 development tools. If you're on something like RISC-V
where Qt WebEngine isn't around, make sure you've got `libwebkit2gtk-4.1-dev`
and `libgtk-3-dev` installed so we can use the fallback engine.

### How to Build

Note: This is for binary builds, for AppImage builds you can use
prebuilt images or utilize `./scripts/build-release.sh`, the dependencies should
be installed automatically on debian systems. It is recommended to build inside
of a debian system or a debian container.

For binary builds, it's pretty simple:

```bash
./scripts/build.sh
```

You'll find the binary at `build/bin/safe-exam-browser`.

### Running an Exam

Just point it at your `.seb` file:

```bash
./safe-exam-browser-qt-x86_64.AppImage /path/to/my-exam.seb
```

Or use a link:

```bash
./safe-exam-browser-qt-x86_64.AppImage sebs://exam-link.seb
```

## Troubleshooting & Support

If you run the app and see a message saying
**"Safe Exam Browser is not supported on your device"**,
it means we couldn't find a compatible browser engine on your system.

We want to fix that!
Please [open an issue here](https://github.com/Jvr2022/seb-linux/issues)
with some details about your system and what version of Linux you're using.

## License

This is an open-source project licensed under the `MPL2`.
Check the `LICENSE` file for the legalese.
