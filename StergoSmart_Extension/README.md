# StergoSmart VS Code Configuration Tool

This folder contains a **local Visual Studio Code extension** that provides a graphical configuration UI for the StergoSmart firmware.  
It allows you to edit `settings.h` using a clean two‑column interface with list selectors instead of dropdowns.

This extension is **not published** on the Marketplace.  
It is meant to be used **locally**, inside your StergoSmart project.

---

## ✨ Features

- Reads current values from `settings.h`
- Displays configuration options in a modern two‑column UI
- Uses list‑style selectors (not dropdowns)
- Automatically highlights the selected option
- Automatically hides **STERGO_PLUG** unless `PROGRAM = 0` or `3`
- Saves changes directly into `settings.h`
- Works in any StergoSmart project folder

---

## 📁 Folder Structure

Place this folder inside your StergoSmart project:

```
StergoSmart/
├── settings.h
└── StergoSmart_Extension/
    ├── extension.js
    ├── package.json
    └── media/
        ├── ui.html
        └── style.css
```

---

## ▶️ How to Run (Developer Mode)

If you want to run the extension without installing it:

1. Open VS Code  
2. Go to: **File → Open Folder**  
3. Select the folder: `StergoSmart_Extension`  
4. Press **F5**

A new VS Code window will open (Extension Development Host).

Inside that window:

- Open your StergoSmart project folder  
- Press **Ctrl + Shift + P**  
- Run: **StergoSmart: Configure**

---

## 📦 How to Package the Extension (.vsix)

If you want to install the extension normally:

### 1. Install `vsce` (only once)

```
npm install -g @vscode/vsce
```

### 2. Package the extension

Inside the `StergoSmart_Extension` folder:

```
vsce package
```

This creates a file like:

```
stergosmart-config-0.0.1.vsix
```

---

## 🛠 How to Install the Extension Locally (No Marketplace)

1. Open VS Code  
2. Go to **Extensions**  
3. Click the `⋮` menu  
4. Choose **Install from VSIX…**  
5. Select your `.vsix` file

Done.  
The extension is now installed permanently.

---

## 🚀 How to Use After Installation

1. Open your StergoSmart project (the folder containing `settings.h`)
2. Press **Ctrl + Shift + P**
3. Run:

```
StergoSmart: Configure
```

The configuration UI will open.

---

## 📝 Notes

- The extension **must** be inside its own folder (not mixed with firmware files)
- The UI reads and writes only the following defines:

```
STERGO_PROGRAM
STERGO_SCREEN
STERGO_PROGRAM_BOARD
STERGO_PLUG
DEBUG
```

- If `settings.h` is missing, the extension will show an error

---

## 📄 License

This extension is part of the StergoSmart project.  
You may modify it freely for personal or project use.

