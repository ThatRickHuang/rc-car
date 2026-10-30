# RC car firmware

Two independent PlatformIO projects for the same car:

| Folder | Board | Purpose |
| --- | --- | --- |
| `esp32-cam/` | ESP32-S3 | Camera stream, web interface, and UART commands |
| `rc_car/` | Arduino Uno | Motor control and IMU code |

Each folder has its own `platformio.ini` and builds separately.

## Start on another computer

Install Git, VS Code, and the PlatformIO IDE extension. Clone this repository:

```sh
git clone https://github.com/ThatRickHuang/rc-car.git
```

Open `rc-car/esp32-cam` or `rc-car/rc_car` as a folder in VS Code. Use a separate VS Code window for each project to avoid uploading firmware from the wrong project. Connect the intended board before uploading.

PlatformIO downloads build tools and dependencies on the first build. Generated `.pio` and editor files are excluded from Git.

## Build and upload

From the repository root, with PlatformIO Core available in your terminal:

```sh
# Camera
pio run -d esp32-cam -e esp32-s3
pio run -d esp32-cam -e esp32-s3 -t upload

# Motor controller
pio run -d rc_car -e uno
pio run -d rc_car -e uno -t upload
```

Upload ports are detected automatically. If both boards are connected, specify the intended port with `--upload-port <port>`. Use `pio device list` to list ports. Port names differ between Windows and macOS.

The camera Wi-Fi SSID and password are currently blank in `esp32-cam/src/web_server.cpp`. Configure them locally before use; do not commit real Wi-Fi credentials.

## Switching computers

Before starting work, run `git pull --ff-only`. Before switching computers:

```sh
git add .
git commit -m "Describe your changes"
git push
```

Commit and push on one computer before pulling on the other.
