# ABT Dehydrator Controller

Arduino Mega2560 food dehydrator controller built with PlatformIO.

The current firmware is still a basic blink/serial starter, but the project is being developed as a structured embedded controller with requirements, ADRs, risk tracking, test planning, and reusable agent definitions.

## Start Here

- Project context: [docs/project-context.md](docs/project-context.md)
- Backlog: [docs/backlog.md](docs/backlog.md)
- Requirements draft: [docs/requirements.md](docs/requirements.md)
- Architecture draft: [docs/architecture.md](docs/architecture.md)
- Hardware assumptions: [docs/hardware.md](docs/hardware.md)
- Development environment: [docs/dev-environment.md](docs/dev-environment.md)
- Agent workflow: [docs/agents.md](docs/agents.md)
- Discovery log: [docs/project-discovery-log.md](docs/project-discovery-log.md)

## Current Starter Firmware

- Initializes `Serial` at `115200`.
- Prints `Hello world!` once at startup.
- Blinks `LED_BUILTIN` every second.
- Prints `LED ON` and `LED OFF` as the LED changes state.

## Hardware And Libraries

- MCU target: Arduino Mega 2560.
- Toolchain: PlatformIO with Arduino framework.
- No external libraries are currently required by the blink starter.

## Build And Upload

- Build: `platformio run`
- Upload: `platformio run --target upload`
- Monitor: `platformio device monitor`

For WSL, devcontainer, SSH-agent, and serial port setup details, see [docs/dev-environment.md](docs/dev-environment.md).
