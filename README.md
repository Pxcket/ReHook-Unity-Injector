# Unity Mod Injector

A minimal Windows console injector that waits for a **Unity** game window, then injects every `.dll` it finds in `./Misc/Mods/`. Includes a progress splash, colored console output, a simple ASCII logo, and an optional success sound.

> Written with ❤️ by **Pxcket** • big help from **Z0mb1e**  

---

## ✨ Features

- 🔎 **Auto-detects Unity** (`UnityWndClass`) and grabs the game’s PID
- 📦 **Bulk injection** of all `.dll` files in `./Misc/Mods/`
- 🎛️ **Colored console** helpers (success/warn/error/purple/magenta)
- 🖼️ **ASCII logo** splash + **loading bar**
- 🔔 Optional **success sound** on each successful injection
- 🔐 **UAC elevation** (restarts as admin if needed)

---

## 📁 Code Structure

```
.
├─ core/
│  └─ Logo.h / Logo.cpp            # ASCII logo + color handling
├─ injector/
│  └─ Injector.h / Injector.cpp    # Unity window find + injection
├─ utils/
│  ├─ Console.h / Console.cpp      # console color helpers
│  ├─ LoadingBar.h / LoadingBar.cpp# startup progress bar
│  └─ Process.h / Process.cpp      # process utilities (Toolhelp32)
└─ main.cpp

```
## 📁 Project Structure
```
.
├─ Misc/
│  ├─ Mods/                     # put your .dll mods here
│  └─ success.wav                  # (optional) sound on success
└─ ReHookLoader.exe
```

---

## 🧰 Requirements

- **OS:** Windows 10/11 (x64)
- **Compiler:** MSVC (Visual Studio 2019/2022) or clang-cl
- **SDK:** Windows SDK (for `WinMM`, `Shell32`, `TlHelp32`, etc.)
- **Linker libs:** `winmm.lib`, `shell32.lib`
- **Console:** runs elevated (admin) for `OpenProcess/VirtualAllocEx`

> The code calls `LoadLibraryW` in the target process. Make sure your target game and your mods match **bitness** (x64 ↔ x64).

---

## 🔧 Build (Visual Studio)

1. Open the solution / create one and add the sources.
2. Set **x64** configuration (must match the target game).
3. Add to **Linker → Input → Additional Dependencies**:
   - `winmm.lib`
   - `shell32.lib`
4. Ensure C++17 or later is enabled.
5. Build & run.

---

## ▶️ Usage

1. **Place your mod DLLs** in:  
   `./Misc/Mods/`
2. (Optional) Put a sound file at:  
   `./Misc/success.wav`
3. **Run the injector as Administrator.**  
   It will elevate itself if needed.
4. **Launch or Have your Unity game Open** (or start the injector first—either way works).
5. The injector will:
   - Wait for a visible, non-minimized **Unity** window
   - Display the executable name & PID
   - Inject each `.dll` found in `./Misc/Mods/`
   - Play `success.wav` on success (if present)

---

## 🧠 How It Works (quick tour)

- **Unity detection:**  
  `FindWindowExA` with class name `UnityWndClass` → `IsWindowVisible` → not minimized.
- **Target PID:**  
  `GetWindowThreadProcessId(hwnd, &pid)`
- **Injection:**  
  - `OpenProcess(PROCESS_ALL_ACCESS, …)`
  - `VirtualAllocEx` to write the wide DLL path
  - `WriteProcessMemory` with the path
  - `CreateRemoteThread` calling `LoadLibraryW` inside the target
- **UX sugar:**  
  - `Console.cpp` provides color helpers via `SetConsoleTextAttribute`
  - `LoadingBar.cpp` animates a 0 → 100% progress bar
  - `Logo.cpp` prints the ASCII banner in purple, then resets colors
- **Admin:**  
  `ShellExecuteExA` with `runas` to relaunch elevated

---

## ⚙️ Config Notes

- **Mods directory:**  
  Hardcoded to `./Misc/Mods` relative to the injector’s working directory.
- **Sound path:**  
  `const char* SOUND_PATH = ".\\Misc\\success.wav";`
- **`skCrypt` macro:**  
  Defined as `#define skCrypt(x) x` in `main.cpp` right now.  
  If you use a real string obfuscator, replace that macro with your implementation.

---

## 🧯 Troubleshooting

- **“No mods found.”**  
  Ensure at least one `.dll` is in `./Misc/Mods/`.
- **“Mods folder missing.”**  
  Create `./Misc/Mods` **exactly** with that casing.
- **Unity not detected.**  
  Game must be running and visible (not minimized).
