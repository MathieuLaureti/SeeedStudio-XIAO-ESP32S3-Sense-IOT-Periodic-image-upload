#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

//your network credentials
#define WIFI_SSID ""
#define WIFI_PASS ""

// Deep sleep timer
#define TIME_TO_SLEEP_SEC 1800 
#define TIME_TO_SLEEP_US (TIME_TO_SLEEP_SEC * 1000000ULL)

#define LED_PIN 21

// your server IP and port
const char* server_ip = ""; 
const int server_port = 0;

String upload_url = "http://" + String(server_ip) + ":" + String(server_port) + "/upload";

/**
 * @brief Blink the led x times to signal a fatal error.
 * 
 * This function does not stop the loop
 */
void error_blink(int times) {
  delay(1000);
  for(int i = 0; i < times ; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(200);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
  }
}

/**
 * @brief Connects to the Wi-Fi network.
 * @return 0 on success
 * 1 on connection failure
 */
int connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() > 20000) { 
      return 1; // Wifi Failure
    }
  }
  return 0; // Successfully connected to Wifi
}

/**
 * @brief Initializes the camera with the specified configuration.
 * @return 0 on success,
 * 2 on Camera failure
 */
int initCamera() {
  camera_config_t config;

  config.pin_sccb_sda   = 40;
  config.pin_sccb_scl   = 39;
  config.pin_xclk       = 10;
  config.pin_pclk       = 13;
  config.pin_vsync      = 38;
  config.pin_href       = 47;
  config.pin_d0         = 15;
  config.pin_d1         = 17;
  config.pin_d2         = 18;
  config.pin_d3         = 16;
  config.pin_d4         = 14;
  config.pin_d5         = 12;
  config.pin_d6         = 11;
  config.pin_d7         = 48;
  config.pin_pwdn       = -1;
  config.pin_reset      = -1;
  config.xclk_freq_hz   = 20000000;
  config.ledc_timer     = LEDC_TIMER_0;
  config.ledc_channel   = LEDC_CHANNEL_0;
  config.pixel_format   = PIXFORMAT_JPEG;
  config.frame_size     = FRAMESIZE_UXGA;
  config.jpeg_quality   = 10;             
  config.fb_count       = 1;
  config.fb_location    = CAMERA_FB_IN_PSRAM; 
  config.grab_mode      = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t camera_init_response = esp_camera_init(&config);
  if (camera_init_response != ESP_OK) {
    return 2; // Camera Init failed
  }

  //color fixing
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_wb_mode(s, 0);
  }else{
    return 2; // Color fix failed
  }
  return 0;
}

/**
 * @brief Takes a picture and uploads it to the server.
 * @return 0 on success,
 * 1 on WiFi failure,
 * 2 on Camera capture failure,
 * 3 on Server error
 */
int takeAndUploadPicture() {

  if (WiFi.status() != WL_CONNECTED) {
    return 1; // Wifi Failure
  }
  
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    esp_camera_fb_return(fb); 
    return 2; // Camera capture failure
  }

  HTTPClient http;
  http.begin(upload_url);
  http.addHeader("Content-Type", "image/jpeg"); 
  int http_code = http.POST(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  http.end();

  if (http_code > 0) {
    if (http_code == HTTP_CODE_OK) {
      return 0; // Image sent successfully
    } else {
      return 3; //Server Error
    }
  } else {
    return 3; //HTTP connection Error
  }
}

/**
 * @brief Puts the device into deep sleep for a defined period and display led Error code if an error arise.
 * @param error_code Error code to indicate the reason for sleep.
 */
void sleepTime(int error_code){
  WiFi.disconnect(true);
  digitalWrite(LED_PIN, HIGH); 
  
  if (error_code != 0) {
    error_blink(error_code);
  }

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
  esp_deep_sleep_start();
}

/**
 * @brief setup() now runs on every wake-up.
 */
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  int camera_init_response = initCamera();
  if (camera_init_response != 0) {
    sleepTime(camera_init_response);
  }

  int wifi_connection_response = connectToWifi();
  if (wifi_connection_response != 0) {
    sleepTime(wifi_connection_response);
  }

  int upload_response = takeAndUploadPicture();
  if(upload_response != 0) {
    sleepTime(upload_response);
  }
  sleepTime(0);

}

void loop() {
}