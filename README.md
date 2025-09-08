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

- A **POSIX terminal** (Linux recommended; macOS/xBSD may work with extra setup) (Use WSL if you're on Windows)
- GCC (or Clang) and GNU Make
- **GNU binutils** (ld, ar, objcopy, etc.)
- `cpio` and `gzip` for **building the initramfs**
- **glibc** or **musl**
- **QEMU** for testing/booting
- **Linux kernel image** (`bzImage`) with initramfs support. Linux kernel linux-6.16.4 recommended.
  
⚠️ **Note**: This project is primarily tested on **Linux**. macOS and BSD users may need to install GNU versions of tools (gmake, GNU binutils, GNU cpio) since their default system tools differ. I haven’t tested these platforms personally.

---

## **Getting Started**

Clone the repository:
```
bash 
git clone https://github.com/terra2o/teros.git 
cd ~/teros
```

---

## **Build the initramfs and programs**

(after you cd into teros as the previous instruction say)
```
make
```

---

## **Run in QEMU**

```
qemu-system-x86_64 \
  -kernel ~/linux-6.16.4/arch/x86/boot/bzImage \
  -initrd ~/teros/initramfs.cpio.gz \
  -append "init=/init"
```

---

## **License**
This project is licensed under GNU General Public License v2 (GPL-2.0) — see [LICENSE](https://github.com/terra2o/teros/blob/main/LICENSE) for details
