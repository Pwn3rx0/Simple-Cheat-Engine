# Simple Cheat Engine
A lightweight, console-based memory manipulation tool written in C for Windows. This utility allows users to scan the virtual memory of a target process, filter for specific integer values, and modify or "freeze" those values in real-time.

---

## 🚀 Features

- **Process Enumeration:** Lists all active system processes with their PIDs.
- **Memory Scanning:** Scans committed memory regions for 4-byte integers.
- **Refined Filtering:** Narrow down scan results by filtering previous hits for updated values.
- **Memory Injection:** Write new values directly into the target process's memory space.
- **Value Freezing:** Continuously overwrites a memory address to prevent the target application from changing it.

---

## 🛠️ Technical Overview

The tool leverages the **Windows API** to interact with the OS kernel and external memory spaces:

* **`CreateToolhelp32Snapshot`**: Snapshots the process list.
* **`VirtualQueryEx`**: Queries memory page attributes (State, Protect) to ensure the scanner only touches valid, committed memory.
* **`ReadProcessMemory` / `WriteProcessMemory`**: The core functions used to view and edit the target's data.

---

## 📋 Usage Instructions

### 1. Compilation
Compile the source using a C compiler on Windows (e.g., MinGW, MSVC, or Clion).
> **Note:** If using Visual Studio, the code is already configured with `_CRT_SECURE_NO_WARNINGS`.

### 2. Standard Workflow
1.  **Open Process:** Run the tool, list processes (Option 1) to find your target's PID, and open it (Option 2).
2.  **Initial Scan:** Search for a known value (e.g., your current "Health" or "Gold" count) using Option 3.
3.  **Refine:** Change the value within the target application, then use Option 4 to filter the existing list for the new value.
4.  **Edit:** Once you have narrowed down the address, use Option 6 to change it or Option 7 to "freeze" it.

---

## ⚠️ Requirements & Disclaimer

* **Platform:** Windows Only.
* **Privileges:** Running as **Administrator** is highly recommended to ensure the tool has permission to access the memory of other running applications.
* **Educational Use:** This project is for educational purposes regarding how memory works in a Windows environment. Using this on software with anti-cheat protections may result in a ban or crash.

---

## 📝 License
MIT License - Feel free to use and modify for your own learning!
