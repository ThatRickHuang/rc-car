#include <Arduino.h>   // Arduino core library
#include <WiFi.h>
#include <WebServer.h>

#include "web_server.h"
#include "camera_server.h"

const char* ssid = "";
const char* password = "";

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>RC Car Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <style>
    body {
      font-family: Arial;
      text-align: center;
      margin-top: 50px;
    }

    .controls {
      display: grid;
      grid-template-columns: 80px 80px 80px;
      gap: 10px;
      justify-content: center;
    }

    .key {
      width: 80px;
      height: 80px;
      font-size: 30px;
      border: 2px solid black;
      border-radius: 10px;
      background-color: white;
      cursor: pointer;
    }

    .pressed {
      background-color: gray;
      transform: scale(0.95);
    }

    #w {
      grid-column: 2;
    }

    #a {
      grid-column: 1;
      grid-row: 2;
    }

    #s {
      grid-column: 2;
      grid-row: 2;
    }

    #d {
      grid-column: 3;
      grid-row: 2;
    }

    #status {
      font-size: 24px;
      margin-top: 30px;
      font-weight: bold;
    }
  </style>
  <style>#camera { width: min(100%, 640px); aspect-ratio: 4 / 3; background: #222; object-fit: contain; transform: rotate(180deg); border-radius: 10px; }</style>
</head>

<body>

  <h1>RC Car Control</h1>
  <img id="camera" alt="Live view from the car camera">
  <p id="camera-status" role="status">Connecting to camera...</p>

  <div class="controls">
    <button id="w" class="key">W</button>
    <button id="a" class="key">A</button>
    <button id="s" class="key">S</button>
    <button id="d" class="key">D</button>
  </div>

  <p id="status">Stopped</p>

<script>
  const cameraImage = document.getElementById("camera");
  const cameraStatus = document.getElementById("camera-status");
  const cameraUrl = new URL(window.location.href);
  cameraUrl.port = "81";
  cameraUrl.pathname = "/stream";
  let cameraTimer;
  function nextFrame() {
    if (document.hidden) return;
    cameraUrl.search = "t=" + Date.now();
    cameraImage.src = cameraUrl.href;
    cameraStatus.textContent = "Live stream requested";
  }
  cameraImage.onload = () => {
    cameraStatus.textContent = "Live camera";
  };
  cameraImage.onerror = () => {
    cameraStatus.textContent = "Camera unavailable - retrying. Check ESP32 serial output.";
    cameraTimer = setTimeout(nextFrame, 2000);
  };
  document.addEventListener("visibilitychange", () => {
    clearTimeout(cameraTimer);
    if (!document.hidden) nextFrame();
    else cameraImage.removeAttribute("src");
  });
  nextFrame();

  const keys = new Set();
  let desiredMovement = "stop";
  let sending = false;
  let pending = false;

  async function sendMovement() {
    pending = true;
    if (sending) return;
    sending = true;
    try {
      while (pending) {
        pending = false;
        const controller = new AbortController();
        const timeout = setTimeout(() => controller.abort(), 500);
        try {
          const response = await fetch("/move?direction=" + desiredMovement,
            { signal: controller.signal, cache: "no-store" });
          if (!response.ok) throw new Error("Command rejected");
        } catch (error) {
          document.getElementById("status").innerText = "Connection lost";
          keys.clear();
          document.querySelectorAll(".key").forEach(b => b.classList.remove("pressed"));
          desiredMovement = "stop"; // Require a fresh press after a failed request.
          pending = false;
        } finally {
          clearTimeout(timeout);
        }
      }
    } finally {
      sending = false;
    }
  }
  setInterval(sendMovement, 200); // Refresh the Uno's command timeout.
  function releaseAll() {
    keys.clear();
    document.querySelectorAll(".key").forEach(b => b.classList.remove("pressed"));
    updateMovement();
  }
  window.addEventListener("blur", releaseAll);
  document.addEventListener("visibilitychange", () => {
    if (document.hidden) releaseAll();
  });

  function updateMovement() {

  const w = keys.has("w");
  const a = keys.has("a");
  const s = keys.has("s");
  const d = keys.has("d");

  let movement = "stop";

  if (w && d) {
    movement = "forward_right";
  }

  else if (w && a) {
    movement = "forward_left";
  }

  else if (s && d) {
    movement = "backward_right";
  }

  else if (s && a) {
    movement = "backward_left";
  }

  else if (w) {
    movement = "forward";
  }

  else if (s) {
    movement = "backward";
  }

  else if (d) {
    movement = "pivot_right";
  }

  else if (a) {
    movement = "pivot_left";
  }

  document.getElementById("status").innerText = movement;

  desiredMovement = movement;
  sendMovement();
}



  function pressKey(key) {
    if (keys.has(key)) return; // Ignore keyboard auto-repeat.

    keys.add(key);

    document
      .getElementById(key)
      .classList.add("pressed");

    updateMovement();
  }



  function releaseKey(key) {
    if (!keys.has(key)) return;

    keys.delete(key);

    document
      .getElementById(key)
      .classList.remove("pressed");

    updateMovement();
  }



  // Keyboard controls

  document.addEventListener("keydown", function(event) {

    const key = event.key.toLowerCase();

    if (["w", "a", "s", "d"].includes(key)) {
      pressKey(key);
    }

  });



  document.addEventListener("keyup", function(event) {

    const key = event.key.toLowerCase();

    if (["w", "a", "s", "d"].includes(key)) {
      releaseKey(key);
    }

  });



  // Mouse controls

  ["w", "a", "s", "d"].forEach(function(key) {

    const button = document.getElementById(key);

    button.addEventListener("mousedown", function() {
      pressKey(key);
    });

    button.addEventListener("mouseup", function() {
      releaseKey(key);
    });

    button.addEventListener("mouseleave", function() {
      releaseKey(key);
    });

  });

</script>

</body>
</html>
)rawliteral";

WebServer server(80);

bool validCommand(const String& command) {
    return command == "stop" || command == "forward" || command == "backward" ||
           command == "pivot_left" || command == "pivot_right" ||
           command == "forward_left" || command == "forward_right" ||
           command == "backward_left" || command == "backward_right";
}

void sendCommand(const String& command) {
    if (!validCommand(command)) {
        server.send(400, "text/plain", "Invalid direction");
        return;
    }
    Serial2.println(command); // One newline-terminated command per message.
    Serial.print("UART sent: ");
    Serial.println(command);
    server.send(200, "text/plain", "Sent");
}

void setupWebServer() {
    WiFi.setSleep(false); // Reduce video/control latency while powered by the car.
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    setupCameraServer(); // Control page remains available if camera initialization fails.

    // Serve the web page
    server.on("/", []() {
        server.send(200, "text/html", html);
    });

    // Receive control commands
    server.on("/move", []() {
        sendCommand(server.arg("direction"));
    });

    // Start web server
    server.begin();

    Serial.println("Web server started");
}

void handleClientRequests() {
    server.handleClient();
}

