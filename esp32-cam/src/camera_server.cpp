#include <Arduino.h>
#include <esp_camera.h>
#include <esp_http_server.h>
#include "camera_server.h"

namespace {
httpd_handle_t cameraServer = nullptr;
esp_err_t stream(httpd_req_t* request) {
    httpd_resp_set_type(request, "multipart/x-mixed-replace;boundary=frame");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    while (true) {
        camera_fb_t* frame = esp_camera_fb_get();
        if (!frame) return ESP_FAIL;
        char header[100];
        int size = snprintf(header, sizeof(header),
            "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
            static_cast<unsigned>(frame->len));
        esp_err_t result = httpd_resp_send_chunk(request, header, size);
        if (result == ESP_OK) {
            result = httpd_resp_send_chunk(request,
                reinterpret_cast<const char*>(frame->buf), frame->len);
        }
        esp_camera_fb_return(frame);
        if (result != ESP_OK) return result;
        vTaskDelay(1); // Yield to networking and driving tasks.
    }
}
esp_err_t capture(httpd_req_t* request) {
    camera_fb_t* frame = esp_camera_fb_get();
    if (!frame) {
        httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Camera capture failed");
        return ESP_FAIL;
    }
    httpd_resp_set_type(request, "image/jpeg");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    esp_err_t result = httpd_resp_send(request, reinterpret_cast<const char*>(frame->buf), frame->len);
    esp_camera_fb_return(frame);
    return result;
}
}

bool setupCameraServer() {
    // ELEGOO supplied S3 camera: S3-EYE pin mapping.
    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 11;
    config.pin_d1 = 9;
    config.pin_d2 = 8;
    config.pin_d3 = 10;
    config.pin_d4 = 12;
    config.pin_d5 = 18;
    config.pin_d6 = 17;
    config.pin_d7 = 16;
    config.pin_xclk = 15;
    config.pin_pclk = 13;
    config.pin_vsync = 6;
    config.pin_href = 7;
    config.pin_sccb_sda = 4;
    config.pin_sccb_scl = 5;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 14;
    config.fb_count = psramFound() ? 2 : 1;
    config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
    esp_err_t error = esp_camera_init(&config);
    if (error != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", error);
        return false;
    }
    // Separate task/server keeps image capture out of the driving handler.
    httpd_config_t serverConfig = HTTPD_DEFAULT_CONFIG();
    serverConfig.server_port = 81;
    serverConfig.ctrl_port = 32769;
    serverConfig.lru_purge_enable = true;
    serverConfig.send_wait_timeout = 2;
    error = httpd_start(&cameraServer, &serverConfig);
    if (error == ESP_OK) {
        httpd_uri_t route = {};
        route.uri = "/capture";
        route.method = HTTP_GET;
        route.handler = capture;
        error = httpd_register_uri_handler(cameraServer, &route);
        if (error == ESP_OK) {
            route.uri = "/stream";
            route.handler = stream;
            error = httpd_register_uri_handler(cameraServer, &route);
        }
    }
    if (error != ESP_OK) {
        Serial.printf("Camera server failed: 0x%x\n", error);
        if (cameraServer) httpd_stop(cameraServer);
        cameraServer = nullptr;
        esp_camera_deinit();
        return false;
    }
    Serial.printf("Camera ready: port 81 /stream; PSRAM: %s\n", psramFound() ? "yes" : "no");
    return true;
}
