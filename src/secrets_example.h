//WIFI Credentials
const char *ssid = "";
const char *password = "";

//Server Local IP, PORT and upload endpoint
//This is used to create the full upload URL in main.cpp
//With this exact format : "http:// + {server_local_ip} + ":" + {server_port} + {upload_endpoint}"
//With the example values below, the full URL would be : http://127.0.0.1:8000/upload
const char *server_local_ip = "127.0.0.1";
const int server_port = 8000;
const char *upload_endpoint = "/upload";

//Sleep time in seconds (Time between picture uploads)
const char sleep_time_sec = 1800;