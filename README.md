# esp32_iot

ESP32-based IoT project that reads temperature and humidity data from a DHT11 sensor and sends it to a Go server via HTTP. This project was built with the help of ESP-IDF and FreeRTOS. The data is preserved in a PostgreSQL database. There might be multiple devices sending the data, and each device has its own API key.

## How to run:
### Server
<!-- - go to server directory
```bash
cd server
```
- build the project
```bash
go mod download
go run api/main.go
``` -->
- make sure Docker is installed
- go to server directory
```bash
cd server
```
Run the docker conatiners
```bash
docker compose up
```

### Esp-32
- make sure ESP-IDF is installed
- go to esp32 directory
```bash
cd dht11-http
```
- modify the following defines. `SERVER_IP` is the IP address of the go server. `WIFI_SSID` and `WIFI_PASSWORD` are the ssid and password of the wifi network credentials.
```c
#define SERVER_IP "192.168.0.145"
#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"
```
- build the project
```bash
idf.py build
```
- Upload the project into esp32 board
```bash
idf.py flash -p <esp32 port>
```
