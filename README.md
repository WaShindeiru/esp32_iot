# esp32_iot

A Esp32-based IoT project that reads temperature and humidity data from a sensor DHT11 and sends it to a Go server via http. Esp32 project is built ESP-IDF and FreeRTOS

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
