# esp32_iot

ESP32-based IoT project that reads temperature and humidity data from a DHT11 sensor and sends it to a Go server via HTTP. This project was built with the help of ESP-IDF and FreeRTOS. The data is preserved in a PostgreSQL database. There might be multiple devices sending the data, and each device has its own API key.

## How to run:
### Server
- go to server directory
```bash
cd server
```
- build the project
```bash
go mod download
go run api/main.go
```

### Esp-32
- make sure ESP-IDF is installed
- go to esp32 directory
```bash
cd dht11-http
```
- build the project
```bash
idf.py build
```
- Upload binary into the board
```bash
idf.py flash -p <esp32 port>
```
