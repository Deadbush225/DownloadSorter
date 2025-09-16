## Install Download Sorter

This project ships a simple cross‑platform build. On Linux, use the generated `install/` bundle and `install.sh`. On Windows, use the installer or build from source.

### Supported platforms

- Linux x86_64 (tested on Debian/Ubuntu, Arch, Fedora)
- Windows 10/11 x86_64 (installer provided)

---

## Linux

There are two phases: build the bundle into `install/`, then run the installer.

### 1) Build the install bundle

Requirements (build):

- CMake ≥ 3.19, a C++20 compiler (gcc/clang), Qt 6 (Core, Widgets)

Example packages:

- Debian/Ubuntu: `sudo apt install cmake g++ qt6-base-dev`
- Fedora: `sudo dnf install cmake gcc-c++ qt6-qtbase-devel`
- Arch: `sudo pacman -S cmake gcc qt6-base`

Build steps from the repo root:

```sh
cd src
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build --target install_local
```

This creates the bundle at `./install/` with:

- `Download Sorter` (the binary)
- `manifest.json` (versioning)
- `install.sh` (the installer script)

### 2) Install

Run the installer from the bundle root:

```sh
cd install
./install.sh             # user install (~/.local)
sudo ./install.sh        # system-wide (/usr)
```

After a user install, ensure `~/.local/bin` is in your PATH:

```sh
# zsh
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zprofile

# bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
```

Run the app:

```sh
download-sorter
```

You can also find “Download Sorter” in your desktop application menu.

### Runtime dependencies (Linux)

If you built against system Qt, the installer won’t bundle Qt libraries. On a fresh system, install the Qt 6 base runtime and X11/XCB bits if the app fails to start:

- Debian/Ubuntu: `sudo apt install libqt6widgets6 libqt6core6 libqt6gui6 libxkbcommon-x11-0 libgl1`
- Fedora: `sudo dnf install qt6-qtbase xcb-util xcb-util-wm xcb-util-image xcb-util-keysyms`
- Arch: `sudo pacman -S qt6-base libxkbcommon xcb-util xcb-util-wm xcb-util-image xcb-util-keysyms`

If you prefer a fully self-contained package, consider creating an AppImage/Flatpak in the future.

### Uninstall (Linux)

Remove the installed files run the installer script with the uninstall flag:

```sh
cd install
./install.sh --uninstall -y
```

This removes the installed files from your prefix (default: `~/.local`). Optionally, you can refresh desktop and icon caches:

```sh
command -v update-desktop-database >/dev/null && update-desktop-database ~/.local/share/applications || true
command -v gtk-update-icon-cache >/dev/null && gtk-update-icon-cache -t ~/.local/share/icons/hicolor || true
```

---

## Windows

### Option A: Use the installer

- Use the installer under `windows-installer/` (or download the latest from Releases).
- Run the `.exe` and follow prompts.

### Option B: Build from source

Requirements: CMake, a C++20 compiler (MSVC or MinGW), Qt 6 (Core, Widgets).

Steps (example, from repo root):

```powershell
cd src
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix ..\install
```

On Windows builds, Qt runtime deployment is handled via `windeployqt` during install if found.

---

## Troubleshooting

- App doesn’t appear in menu after Linux install:
  - Ensure `download-sorter.desktop` exists under your prefix’s `share/applications`.
  - Run `update-desktop-database` and `gtk-update-icon-cache` if available.
- “cannot open shared object file … Qt6…” on Linux:
  - Install the Qt 6 base runtime for your distro (see Runtime dependencies above).
  - Or rebuild on the target system and reinstall.
- No icon:
  - Ensure `src/icons/Download Sorter.png` exists before building, or install ImageMagick to convert from `.ico` during `install.sh`.

---

## Notes

- For system-wide installs, the installer uses `/usr` by default; for user installs it uses `~/.local`.
- The Linux bundle is created by the `install_local` CMake target.
- Built artifacts and installer scripts live in `install/` after a successful build.
