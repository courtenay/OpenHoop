# Contributing to OpenHoop

Thank you for your interest in contributing to OpenHoop! We’re building this project together — whether you’re a hardware tinkerer, software developer, or artist, your ideas matter.

This guide is here to make contributing as smooth as possible. If anything is unclear, open an issue or start a discussion — we’re happy to help.

---

## How to Get Started

1. **Explore the project**
   * Read the [README](README.md) for an overview of the hardware and firmware.
   * Browse open issues and discussions to see where help is needed most.
2. **Pick a task**
   * Comment on the issue you want to work on so others know it’s in progress.
   * If your idea is new, open an issue to outline what you plan to build or fix.
3. **Sync with the community**
   * Join our chat (see the README for links) or start a GitHub Discussion if you would like feedback early.
4. **Set up your environment** using the steps below, then start coding!

---

## Local Setup Guide

### 1. PlatformIO Setup

OpenHoop’s firmware is organized as a [PlatformIO](https://platformio.org/) project.

1. Install PlatformIO:
   * **VS Code Extension:** Install the “PlatformIO IDE” extension and restart VS Code.
   * **CLI:** Run `pip install platformio` (or use the installer for your OS).
2. Clone the repository and install dependencies:
   ```bash
   git clone https://github.com/<your-username>/OpenHoop.git
   cd OpenHoop
   pio run
   ```
   The first `pio run` downloads required frameworks and libraries.
3. Build and upload:
   * Build only: `pio run`
   * Build & upload: `pio run -t upload`
   * Monitor serial: `pio device monitor`

### 2. Arduino IDE Setup (Optional)

If you prefer the Arduino IDE:

1. Install the Arduino IDE (1.8+ or 2.x).
2. Add the appropriate board support package (see `platformio.ini` for board details).
3. Install libraries listed in `lib/` and `platformio.ini` manually through the Library Manager if they are not bundled.
4. Open the `src/` sketch files and ensure the board and port match your hoop controller.

> **Tip:** PlatformIO provides automated dependency management and reproducible builds, so we recommend it for most contributors.

### 3. Connecting to the Hoop & Viewing Serial Output

1. Connect the hoop controller to your computer via USB.
2. Identify the serial port:
   * macOS/Linux: run `ls /dev/tty.*` or `ls /dev/ttyUSB*`.
   * Windows: check the COM port in Device Manager.
3. Start a serial monitor:
   * PlatformIO: `pio device monitor`
   * Arduino IDE: Tools → Serial Monitor (set the baud rate to match the firmware, typically `115200`).
4. Reset the device (if needed) to view boot messages and runtime logs.

---

## Code Style Guidelines

### General Principles

* Favor clarity over cleverness — optimize for readability.
* Keep functions focused. Break large routines into smaller helpers when they exceed ~100 lines.
* Write unit or integration tests when practical (see the `test/` directory and PlatformIO docs).

### Naming Conventions

| Element              | Convention             | Example            |
| -------------------- | ---------------------- | ------------------ |
| C++ variables        | `snake_case`           | `led_brightness`   |
| C++ functions        | `snake_case`           | `update_animation` |
| C++ classes/structs  | `PascalCase`           | `AnimationFrame`   |
| Constants/macros     | `ALL_CAPS`             | `MAX_LEDS`         |
| File names           | `snake_case`           | `animation_driver.cpp` |

### Formatting

* **Indentation:** 2 spaces (no tabs) for C++ and Arduino code.
* **Line length:** Aim for ≤100 characters where practical.
* **Braces:** Place the opening brace on the same line as the control statement or function.
* **Includes:** Group standard library headers, third-party headers, then project headers. Separate groups with a blank line.

### Commenting (C++ / Arduino)

* Use `//` for inline explanations and `/* ... */` only for block comments that span multiple lines.
* Document non-obvious logic and hardware-specific assumptions.
* Public functions should have a brief `///` Doxygen-style comment describing purpose, inputs, and outputs when they are part of a shared interface.

---

## Commit Message Guidelines

Structured commits help reviewers understand your changes.

* **Format:** `type(scope): short summary`
  * Examples: `fix(animations): guard against empty frame list`, `docs: update contributing guide`.
* **Types:** `feat`, `fix`, `docs`, `refactor`, `test`, `chore`, or another short descriptor.
* **Body (optional but encouraged):** Wrap at 72 characters, explain the motivation and key changes.
* **Footer:** Reference related issues using `Fixes #123` or `Refs #456` when applicable.

Commit early and often, but keep each commit focused on a single logical change.

---

## Pull Request Process

Use this checklist to keep PRs easy to review:

- [ ] Ensure the branch is up to date with `main`.
- [ ] Build the firmware (`pio run`) and run any relevant tests.
- [ ] Update documentation, diagrams, or configuration files if the behavior changes.
- [ ] Provide screenshots, serial logs, or videos when the change impacts user-visible behavior.
- [ ] Fill out the PR template, summarizing what changed and why.
- [ ] Request a review from a maintainer and respond to feedback promptly.

Once approved, a maintainer will merge the PR. If you have merge permissions, wait for at least one approval before merging.

---

## Code of Conduct

We follow the Contributor Covenant to ensure a welcoming environment. By participating in OpenHoop you agree to:

* Use welcoming and inclusive language.
* Be respectful of differing viewpoints and experiences.
* Accept constructive criticism gracefully.
* Focus on what is best for the community.
* Show empathy toward other community members.

Unacceptable behavior includes harassment, trolling, unwelcome sexual attention, public or private harassment, or publishing others’ private information without permission. Instances of abusive, harassing, or otherwise unacceptable behavior may be reported by contacting the project team at **openhoop@gmail.com**. Project maintainers will review all reports and take appropriate action.

Let’s build something spectacular together — thank you for contributing!
