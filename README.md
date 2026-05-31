# SimpleBiquadLPF 🎛️🌐

A lightweight, modern **React + Vite WebView-based JUCE VST3 Plugin Template** featuring a hard-coded Biquad Low-Pass Filter. This repository is production-ready and highly optimized for instantaneous UI/UX prototyping and audio debugging.

---

## 🚀 Key Features & Architecture

### 1. Modern Web UI Integrated via JUCE 8 WebView
*   **Tech Stack:** Built with `React`, `Vite`, and `TypeScript`, styled beautifully using `shadcn@latest`.
*   **Custom React Hooks for JUCE Interop:** Includes custom-built hooks designed to seamlessly sync frontend components with JUCE parameters:
    *   `useJuceSlider` / `useJuceKnob` (with a built-in knob drag bugfix)
    *   `useJuceToggle`
    *   `useJuceComboBox`
*   **Robust Asset Handling:** Implemented efficient webview file handling, resource retrieval, and optimized static asset caching (via ZIP prefix resolution) inside `PluginEditor`.

### 2. Audio & DSP Engine (C++)
*   **Biquad LPF Implementation:** Features a manually controllable Biquad Filter with dedicated parameters for both **Cutoff Frequency** and **Resonance**, fully bound to `APVTS` (AudioProcessorValueTreeState).
*   **DSP Stability & Smoothing:** Implemented frequency smoothing algorithms and stability checks in the filter processing loop to avoid zipper noise and audio artifacts.
*   **JUCE 8 Ready:** Includes precise version hints for `AudioParameter` declarations to explicitly resolve JUCE 8 `jassert` behaviors.

### 3. Developer Experience (DX) & Debugging
*   **Instant Visual Feedback:** Optimized for rapid local debugging inside Visual Studio Code.
*   **AudioPluginHost Integration:** Includes a pre-configured `filtergraph` file for the JUCE AudioPluginHost, allowing you to instantly load, route, and test the plugin right after compilation.
*   **Modern Build System:** Full `CMake` architecture for seamless cross-platform building and dependency management.

---

## 🛠️ Tech Stack Summary

| Layer | Technologies Used |
| :--- | :--- |
| **Audio Core** | C++, JUCE 8, CMake, APVTS |
| **Frontend UI** | React, Vite, TypeScript, Tailwind CSS, Shadcn UI |
| **Bridge** | JUCE WebView Component, Custom Native-JS Interop Layer |

---

## 📂 Project History & Milestones

*   **Phase 1 (April):** Core `SimpleFilterChannel` DSP implementation based on a pure CMake/JUCE skeleton.
*   **Phase 2 (Early May):** Audio engine stability upgrades (Frequency smoothing, parameter bug fixes, and JUCE 8 compatibility adjustment).
*   **Phase 3 (Mid May):** Frontend architecture overhaul. Integrated `shadcn UI`, established the native `Juce` object bridge, and implemented robust custom hooks for seamless state synchronization.
*   **Phase 4 (Late May):** Optimized asset loading/linking performance via static cache mechanisms and robust resource retrieval in the `PluginEditor`.

---
## 🤝 Connect & Contributing
Developed by **Jiho** - Sophomore studying Computer Science & Mathematics at Seoul National University.

Contributions for adding other frontend frameworks (Vue, Svelte, etc.) or enhancing the DSP pipeline are welcome. Please open an issue or submit a Pull Request!