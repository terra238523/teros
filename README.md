# **TerOS** 🐍🖥️

**TerOS** is a tiny hobby operating system written in **C**.  
It has it's own **custom init shell** (`init.c`) and a small app (`snake.c`).  
This project is designed for learning OS development, static binaries, and basic kernel/initramfs concepts.

---

## **Included Programs**

- **init** — the main shell
- **snake** — classic Snake game
- **Built-in commands** in `init`:
  - `ls` — list files
  - `cat <file>` — display file contents
  - `save <file>` — create/edit a file
  - `cd <dir>` — change directory
  - `pwd` — show current directory
  - `exit` — exit shell
- External programs can be added in `/bin` or `/sbin`. Just compile them statically.

---

## **Features**

- Fully **static binaries**, no external library dependencies
- Bootable in **QEMU**
- Simple **init shell** with process management
- Easy to extend with your own small C apps

---

## **Requirements**

- **POSIX Terminal (Linux, xBSD, macOS etc.)** (Use WSL if on Windows.)
- Some kernel. I personally used Linux 
- **GCC** for compilation
- **QEMU** for testing and booting

---

## **Getting Started**

Clone the repository:

```bash
git clone https://github.com/YOUR_USERNAME/myos.git
cd myos ```
