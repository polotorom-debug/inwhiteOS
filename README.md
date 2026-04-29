<div align="center">

# 🖥️ In White OS

<img src="https://img.shields.io/badge/OS-Experimental-blue?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Architecture-x86-red?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Language-C%20%7C%20Assembly-yellow?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Status-Early%20Development-orange?style=for-the-badge"/>

<br><br>

<img src="https://upload.wikimedia.org/wikipedia/commons/3/35/Tux.svg" width="120"/>

</div>

---

## 📌 Overview

**In White OS** is a custom **x86 operating system** developed from scratch for educational purposes.  
The project focuses on understanding low-level system programming, boot processes, and operating system internals.

It is inspired by early systems such as :contentReference[oaicite:0]{index=0} and educational operating systems like :contentReference[oaicite:1]{index=1}.

---

## ⚙️ Features

- 🔹 Custom x86 Bootloader (Assembly)
- 🔹 32-bit Protected Mode Kernel
- 🔹 Interrupt Descriptor Table (IDT)
- 🔹 Programmable Interrupt Controller (PIC)
- 🔹 Timer Interrupts (PIT)
- 🔹 VGA Text Mode Driver
- 🔹 PS/2 Keyboard Driver
- 🔹 Basic Memory Management (kmalloc)
- 🔹 Simple Shell Interface
- 🔹 Early Filesystem Layer

---

## 🧠 System Architecture


[ Bootloader ]
↓
[ Protected Mode Transition ]
↓
[ Kernel Initialization ]
↓
[ Drivers (VGA / Keyboard / Timer) ]
↓
[ Shell Interface ]



## 🚀 Current Status

> ⚠️ Early development stage (v0.01.1)

The system is currently capable of:
- Booting via custom bootloader
- Entering protected mode
- Initializing hardware subsystems
- Running a basic shell environment

---

## 💻 Technologies

- C (Kernel development)
- x86 Assembly (Bootloader & low-level routines)
- BIOS interrupts (early boot stage)
- VGA text mode (display system)
- Hardware-level programming

---

## 🎯 Project Goal

The goal of **In White OS** is to fully understand how operating systems work internally:

- Boot process (BIOS → kernel)
- Memory management (paging & heap)
- Process scheduling
- Hardware communication
- System calls
- Filesystem implementation

---

## 🛣️ Roadmap

- [x] Bootloader
- [x] Kernel entry
- [x] Interrupt system
- [x] Basic drivers
- [ ] Paging (virtual memory)
- [ ] Multitasking scheduler
- [ ] User mode (Ring 3)
- [ ] ELF binary loader
- [ ] Advanced filesystem
- [ ] Networking stack

---

## ⚠️ Disclaimer

This project is purely **educational and experimental**.  
It is not intended for production or real-world use.

---

## 👨‍💻 Author

Developed independently as a systems programming learning project.

---

<div align="center">

⭐ If you like this project, consider starring it!

</div>
