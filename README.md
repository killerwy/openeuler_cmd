# openEuler_cmd

> An openEuler-compatible filesystem command interface for Windows operating systems.

[English](./README.md) | [中文](./README_zh.md)

![GitHub stars](https://img.shields.io/github/stars/killerwy/openEuler_cmd?style=for-the-badge&logo=github) ![GitHub forks](https://img.shields.io/github/forks/killerwy/openEuler_cmd?style=for-the-badge&logo=github) ![GitHub issues](https://img.shields.io/github/issues/killerwy/openEuler_cmd?style=for-the-badge&logo=github) ![Last commit](https://img.shields.io/github/last-commit/killerwy/openEuler_cmd?style=for-the-badge&logo=github) ![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)

## 📑 Table of Contents

- [Description](#-description)
- [Key Features](#-key-features)
- [Use Cases](#-use-cases)
- [Quick Start](#-quick-start)
- [Project Structure](#-project-structure)
- [Command List](#-command-list)
- [License](#-license)

## 📝 Description

openEuler_cmd is an open-source utility designed to bridge the gap between Windows and openEuler environments. By providing a command interface compatible with openEuler filesystem commands directly on the Windows operating system, it allows developers and system administrators to maintain a consistent workflow across differing platforms without relying on heavy virtualization.

The project is organized into modular components, separating its functional core in the openEuler_cmd_core directory from its user interface and execution window logic in openEuler_cmd_window. The entire workspace is managed and built using the openEuler_cmd.slnx Visual Studio solution file, facilitating standard development and debugging on Windows machines.

## ✨ Key Features

- **🪟 Windows Command Compatibility** — Enables openEuler filesystem commands to be executed natively within Windows environments.
- **🏗️ Modular Project Separation** — Splits core command logic from the window wrapper using separate openEuler_cmd_core and openEuler_cmd_window directories.
- **💻 Visual Studio Solution Integration** — Provides an openEuler_cmd.slnx solution file for unified compilation and development in Windows IDEs.

## 🎯 Use Cases

- Developers working on Windows who need to write and test scripts containing openEuler-compatible filesystem operations.
- System administrators looking to maintain command-line tool consistency when transitioning between Windows environments and openEuler servers.

## ⚡ Quick Start

1. Clone the repository

```bash

git clone https://github.com/killerwy/openEuler_cmd.git
cd openEuler_cmd

```

2. Build the openEuler_cmd_core project to generate the static library openEuler_cmd_core.lib.

3. Build and run the openEuler_cmd_window project. It will launch the console application and generate the executable file openEuler_cmd_window.exe.

## 📁 Project Structure

```
.
openEuler_cmd/
├── openEuler_cmd_core/          # Core module: Implements openEuler commands & Windows API integration
│   ├── openEuler_cmd_core.h     # Core header: Data structures, error codes, function declarations
│   ├── openEuler_cmd_core.c     # Core source: ls/cat/cp/rm/mv/mkdir/rmdir/cd/find implementation
│   ├── openEuler_cmd_core.vcxproj          # Core module VS project config
│   └── openEuler_cmd_core.vcxproj.filters  # Core module VS project filters
│
├── openEuler_cmd_window/        # UI module: Input processing, command parsing, result display
│   ├── openEuler_cmd_window.cpp # Main entry: Command splitting, parsing, dispatching, terminal interaction
│   ├── openEuler_cmd_window.vcxproj          # UI module VS project config
│   └── openEuler_cmd_window.vcxproj.filters  # UI module VS project filters
│
├── openEuler_cmd.slnx           # VS solution: Manages core & window sub-projects
└── README.md                    # Project docs: Features, environment, usage
```

## 🖥 Command List
# openEuler Compatible Command Line - Command Reference
| Command | Core Function | Supported Options | Option Description |
|---------|---------------|-------------------|--------------------|
| ls      | List directory contents (files and subdirectories) | -a | Display all items, including hidden files (starting with `.`) |
|         |               | -l | Long format output, showing details like type, size and modification time |
|         |               | -h | Used with `-l`, convert file size to human-readable format (KB/MB/GB) |
| cat     | Read and output file content | -n | Add line numbers to all lines (including blank lines) |
|         |               | -b | Add line numbers only to non-blank lines |
|         |               | -s | Squeeze multiple consecutive blank lines into one |
| cp      | Copy files or directories | -r/-R | Recursively copy directories (including subdirectories and files) |
| rm      | Delete files or directories | -r/-R | Recursively delete directories (including subdirectories and files) |
| mv      | Move or rename files/directories | — | No additional options |
| mkdir   | Create directories | -p | Create parent directories recursively (auto-generate missing upper directories) |
| rmdir   | Delete empty directories | -p | Recursively delete empty parent directories (only empty directories are allowed) |
| cd      | Change the current working directory | — | No additional options |
| find    | Recursively search for specified files | — | No additional options |
| help    | View command help information | Command name | Enter `help [command]` to check single command usage; enter `help` directly for all commands |
| exit    | Exit the command-line program | — | No additional options |

## 📜 License

This project is licensed under the **MIT** License.

---
*This README was generated with ❤️ by [ReadmeBuddy](https://readmebuddy.com)*
