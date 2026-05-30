# PlatformIO Arduino Blink Stub

Minimal Arduino/PlatformIO starter project.

Overview
- Initializes `Serial` at `115200`.
- Prints `Hello world!` once at startup.
- Blinks `LED_BUILTIN` every second.
- Prints `LED ON` and `LED OFF` as the LED changes state.

Hardware & Libraries
- MCU: Arduino Mega 2560 (default PlatformIO env)
- Optional targets: Arduino Uno and Arduino Nano ATmega328
- No external libraries are required.

Build & Upload (PlatformIO)
- Prerequisites: PlatformIO Core or VS Code with PlatformIO extension.
- Build: `platformio run`
- Upload: `platformio run --target upload`
- Monitor: `platformio device monitor`

Dev Container (VS Code + WSL)
- This repository includes a `.devcontainer/` setup for a Linux-based PlatformIO workflow inside VS Code.
- Open the project from WSL with `code .`, then run `Dev Containers: Reopen in Container`.
- The container installs PlatformIO Core globally. The workspace's `.devcontainer/devcontainer.json` forwards the WSL host SSH agent into the container by mounting `${localEnv:SSH_AUTH_SOCK}` at `/ssh-agent`.
- For GitHub over SSH, the SSH key must be available to the WSL host agent before the container is created or rebuilt. Configure WSL with a stable agent socket in `~/.bashrc`:
  ```bash
  export SSH_AUTH_SOCK="$HOME/.ssh/ssh-agent.sock"

  ssh-add -l >/dev/null 2>&1
  ssh_status=$?

  if [ "$ssh_status" -eq 2 ]; then
    rm -f "$SSH_AUTH_SOCK"
    eval "$(ssh-agent -a "$SSH_AUTH_SOCK" -s)" >/dev/null
    ssh_status=1
  fi

  if [ "$ssh_status" -eq 1 ] && [ -t 0 ] && [ -f "$HOME/.ssh/id_ed25519" ]; then
    ssh-add "$HOME/.ssh/id_ed25519"
  fi
  ```
- Open a new WSL terminal or run `source ~/.bashrc`, then verify the host agent:
  - `echo "$SSH_AUTH_SOCK"` should print `/home/<user>/.ssh/ssh-agent.sock`
  - `ssh-add -l` should list your GitHub SSH key
  - `ssh -T git@github.com` should authenticate to your GitHub account
- Start VS Code from that same WSL environment with `code .`, then reopen or rebuild the container.
- Inside the dev container, verify that the forwarded agent is visible:
  - `echo "$SSH_AUTH_SOCK"` should print `/ssh-agent`
  - `ssh-add -l` should list the same key
  - `ssh -T git@github.com` should authenticate to your GitHub account
- If `ssh-add -l` works in WSL but not inside the container, close VS Code, open a fresh WSL terminal, verify `ssh-add -l` again, start VS Code with `code .`, and rebuild the container.
- If your key only exists in the Windows agent, bridge that agent into WSL first or copy the SSH keypair you intend to use into WSL and load it there.
- Common commands inside the container:
  - `platformio run`
  - `platformio run --target upload --upload-port /dev/ttyUSB0`
  - `platformio device monitor --port /dev/ttyUSB0`

Serial Port Access From Windows Host To Dev Container Running In WSL
- Recommended path: attach the USB serial adapter to WSL first, then pass the resulting Linux device into the dev container.
- Install `usbipd-win` on Windows if it is not already present.
- In an elevated PowerShell on Windows, list USB devices:
  - `usbipd list`
- Find the USB-to-serial adapter, then bind and attach it to your WSL distro:
  - `usbipd bind --busid <BUSID>`
  - `usbipd attach --wsl --busid <BUSID>`
- Back in WSL, confirm the device name:
  - `ls /dev/ttyUSB* /dev/ttyACM*`
- Stop any running dev container for this workspace, then add the device mapping to `.devcontainer/devcontainer.json`:
  - `"runArgs": ["--init", "--device=/dev/ttyUSB0:/dev/ttyUSB0"]`
- Rebuild the container with `Dev Containers: Rebuild Container`.
- Inside the container, verify access:
  - `ls -l /dev/ttyUSB0`
  - `platformio device list`
- Upload using the same port path:
  - `platformio run --target upload --upload-port /dev/ttyUSB0`

Notes
- Replace `/dev/ttyUSB0` with the actual device shown in WSL. Some adapters appear as `/dev/ttyACM0`.
- Direct access to a Windows `COMx` device from a Linux dev container is not native. The stable approach is USB passthrough into WSL with `usbipd`, then Docker device passthrough into the container.
- If the adapter number changes after reconnecting, update the `--device=...` entry and rebuild the container.
