# openEuler_cmd

> 一款面向 Windows 操作系统的 openEuler 兼容文件系统命令接口工具

[English](./README.md) | [中文](./README_zh.md)

![GitHub stars](https://img.shields.io/github/stars/killerwy/openEuler_cmd?style=for-the-badge&logo=github)
![GitHub forks](https://img.shields.io/github/forks/killerwy/openEuler_cmd?style=for-the-badge&logo=github)
![GitHub issues](https://img.shields.io/github/issues/killerwy/openEuler_cmd?style=for-the-badge&logo=github)
![Last commit](https://img.shields.io/github/last-commit/killerwy/openEuler_cmd?style=for-the-badge&logo=github)
![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)
![C](https://img.shields.io/badge/c-%23555555.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/c++-%23F34B7D.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

## 📑 目录

- [项目介绍](#-项目介绍)
- [核心特性](#-核心特性)
- [适用场景](#-适用场景)
- [技术栈](#-技术栈)
- [快速开始](#-快速开始)
- [项目结构](#-项目结构)
- [命令列表](#-命令列表)
- [许可证](#-许可证)

## 📝 项目介绍

openEuler_cmd 是一款开源工具，旨在打通 Windows 与 openEuler 环境之间的使用壁垒。通过在 Windows 操作系统上直接提供兼容 openEuler 文件系统命令的交互接口，开发者和系统管理员无需依赖重量级虚拟化方案，即可在不同平台间保持一致的工作流程。

项目采用模块化组件设计：核心功能逻辑集中在 `openEuler_cmd_core` 目录，用户界面与执行窗口逻辑则放在 `openEuler_cmd_window` 目录。整个工作区通过 `openEuler_cmd.slnx` Visual Studio 解决方案文件进行管理和构建，便于在 Windows 设备上开展标准化的开发与调试工作。

## ✨ 核心特性

- **🪟 Windows 命令兼容** — 支持在 Windows 环境中原生执行 openEuler 文件系统相关命令。
- **🏗️ 模块化项目拆分** — 将核心命令逻辑与窗口封装层分离，分别置于 `openEuler_cmd_core` 和 `openEuler_cmd_window` 目录。
- **💻 Visual Studio 解决方案集成** — 提供 `openEuler_cmd.slnx` 解决方案文件，支持在 Windows 集成开发环境中统一编译和开发。

## 🎯 适用场景

- 工作在 Windows 系统下，需要编写并测试包含 openEuler 兼容文件系统操作脚本的开发者。
- 在 Windows 环境与 openEuler 服务器之间切换时，希望保持命令行工具使用一致性的系统管理员。

## 🔧 技术栈

- 编程语言：C、C++
- 库与应用程序接口：Standard C Library、C++ STL、Win32 Native API

## ⚡ 快速开始

1. 克隆代码仓库

```bash
git clone https://github.com/killerwy/openEuler_cmd.git
cd openEuler_cmd
```

2. 构建 `openEuler_cmd_core` 项目，生成静态库文件 `openEuler_cmd_core.lib`。

3. 构建并运行 `openEuler_cmd_window` 项目，该项目会启动控制台应用程序并生成可执行文件 `openEuler_cmd_window.exe`。

## 📁 项目结构

```
openEuler_cmd/
├── openEuler_cmd_core/          # 核心模块：实现 openEuler 命令及 Windows API 集成
│   ├── openEuler_cmd_core.h     # 核心头文件：数据结构、错误码、函数声明
│   ├── openEuler_cmd_core.c     # 核心源文件：openEuler 命令实现
│   ├── openEuler_cmd_core.vcxproj          # 核心模块 VS 项目配置文件
│   └── openEuler_cmd_core.vcxproj.filters  # 核心模块 VS 项目筛选器配置
│
├── openEuler_cmd_window/        # 界面模块：输入处理、命令解析、结果展示
│   ├── openEuler_cmd_window.cpp # 主入口文件：命令拆分、解析、分发、终端交互逻辑
│   ├── openEuler_cmd_window.vcxproj          # 界面模块 VS 项目配置文件
│   └── openEuler_cmd_window.vcxproj.filters  # 界面模块 VS 项目筛选器配置
│
├── openEuler_cmd.slnx           # VS 解决方案文件：管理核心和界面子项目
├── README.md                    # 英文项目说明文档
├── README_zh.md                 # 中文项目说明文档
├── LICENSE                      # 项目开源许可证文件
└── .gitignore                   # Git 版本控制忽略文件配置
```

## 🖥 命令列表
### openEuler 兼容命令行 - 命令参考
| 命令   | 核心功能                 | 支持的选项 | 选项说明                                   |
|--------|--------------------------|------------|--------------------------------------------|
| ls     | 列出目录内容（文件和子目录） | -a         | 显示所有项目，包括隐藏文件（以 `.` 开头）|
|        |                          | -l         | 长格式输出，显示类型、大小、修改时间等详情 |
|        |                          | -h         | 配合 `-l` 使用，将文件大小转换为易读格式（KB/MB/GB） |
| cat    | 读取并输出文件内容       | -n         | 为所有行添加行号（包括空行）|
|        |                          | -b         | 仅为非空行添加行号                         |
|        |                          | -s         | 将连续的多个空行压缩为一个                 |
| cp     | 复制文件或目录           | -r/-R      | 递归复制目录（包含子目录和文件）|
| rm     | 删除文件或目录           | -r/-R      | 递归删除目录（包含子目录和文件）|
| mv     | 移动或重命名文件/目录    | —          | 无额外选项                                 |
| mkdir  | 创建目录                 | -p         | 递归创建父目录（自动生成缺失的上层目录）|
| rmdir  | 删除空目录               | -p         | 递归删除空的父目录（仅允许删除空目录）|
| cd     | 切换当前工作目录         | —          | 无额外选项                                 |
| find   | 递归搜索指定文件         | —          | 无额外选项                                 |
| help   | 查看命令帮助信息         | 命令名称   | 输入 `help [命令名]` 查看单个命令用法；直接输入 `help` 查看所有命令 |
| exit   | 退出命令行程序           | —          | 无额外选项                                 |

## 📜 许可证

本项目基于 **MIT** 许可证开源。

---
*本 README 由 [ReadmeBuddy](https://readmebuddy.com) 倾情生成 ❤️*