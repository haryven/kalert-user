# kalert-user

Kalert user space library and daemon program

## Installation

```bash
make
sudo make install
```

## Usage in your application

```c
#include <libkalert/libkalert.h>

int main() {
    kalert_msg(LOG_INFO, "Hello from my app");
    return 0;
}
```

## Compile your application using pkg-config

```bash
gcc your_app.c $(pkg-config --cflags --libs libkalert) -o your_app
```

## Project Structure

```
${PROJECT_NAME}/
├── include/ # public headers
├── libkalert/ # library source code
├── src/ # daemon and example applications
├── build/ # build artifacts
└── Makefile
```
