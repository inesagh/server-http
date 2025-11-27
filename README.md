# Server HTTP – C Multithreaded Server Application

This server handles **GET**, **POST**, and **ECHO** requests using the custom Http protocol.
It is multithreaded (each client runs in its own pthread) and processes incoming requests using a layered architecture.
---


### Layer Overview

| Layer      | Responsibility                                  |
| ---------- | ----------------------------------------------- |
| **core**   | Socket creation, bind/listen, threading logic   |
| **rest**   | Parse requests, build responses, route handlers |
| **model**  | RequestLine, StatusLine, headers, models        |
| **util**   | Logging, file I/O helpers, error codes          |
| **main.c** | Server entrypoint                               |

---

## Building 
Note: Make sure you've installed GCC

### 1. Clean previous builds

```bash
rm -f *.o libserver.a
```

### 2. Compile all modules into .o files
```bash
gcc -c src/core/server.c -o server.o \
    -I./src -I./src/core -I./src/rest -I./src/rest/model -I./src/util

gcc -c src/core/thread.c -o thread.o \
    -I./src -I./src/core -I./src/rest -I./src/rest/model -I./src/util

gcc -c src/rest/http_server.c -o http_server.o \
    -I./src -I./src/core -I./src/rest -I./src/rest/model -I./src/util

gcc -c src/rest/parser.c -o parser.o \
    -I./src -I./src/core -I./src/rest -I./src/rest/model -I./src/util

gcc -c src/util/files.c -o files.o \
    -I./src -I./src/core -I./src/rest -I./src/rest/model -I./src/util

gcc -c src/util/logger.c -o logger.o \
    -I./src -I./src/core -I./src/rest -I./src/rest/model -I./src/util
```

### 3. Archive object files into a static library
```bash
ar rcs libserver.a server.o thread.o http_server.o parser.o logger.o files.o
```

### 4. Compile main.c
```bash
gcc -c src/main.c -o main.o \
    -I./src -I./src/core -I./src/rest -I./src/rest/model -I./src/util
```

### 5. Link everything into the final executable
```bash
gcc main.o -L. -lserver -o server -lpthread 
```

### Your executable will appear as:
```bash
./server
```

## Running the Server
### Start the server:
```bash
./server
```

### Expected output:
```bash
[INFO] Server listening on port 8080
```
The server will accept multiple clients simultaneously — each connection is processed in a dedicated thread.

### How It Works
**GET**
Reads a file from the assets/ directory and sends it back.
**POST**
Saves the request body into a uniquely generated file under assets/.
**ECHO**
Returns the body exactly as received.


#### Testing with the Client
```bash
./client GET 127.0.0.1 8080 /index.txt
./client POST 127.0.0.1 8080 /upload "Hello Server"
./client ECHO 127.0.0.1 8080 "echo me back"
```

### Example Server Log
```bash
[02:48:53] [INFO] Server listening on port 8080
[02:48:56] [INFO] Accepted new client with fd=4


--------------------
REQUEST:
Request-Line: POST /upload CHLP/1.0

Headers:
  Body-Size: 12

Body (12 bytes):
Hello Server
--------------------

[02:48:56] [INFO] Handling POST /upload
[02:48:56] [INFO] POST body received (12 bytes): Hello Server

--------------------
RESPONSE:
Status: CHLP/1.0 200 OK

Headers:
  Body-Size: 31

Body (31 bytes):
POST data received successfully
--------------------

[02:48:56] [INFO] Client connection closed
```