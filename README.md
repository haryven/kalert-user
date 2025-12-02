# kalert-user

Kalert user space library and daemon program

## 1.Compile and install
- **Using Makefile**

```bash
make
sudo make install
# This will compile the source code and install the binaries
# to the default system path (usually /usr/local/bin).
```
- **Using rpmbuild**

```bash
rpmbuild -ba packaging/kalert-user.spec
# The resulting RPM can be found under the RPMS directory.
```
## 2.kalertd - Kernel Fault Event Alert Daemon

kalertd is a user-space daemon that listens for kernel fault alerts
and logs them. It also handles initialization of kalert channel
parameters and supports dynamic reloading of configuration files.

### Configuration
The main configuration file is located at:
```bash
/etc/kalertd/kalertd.conf
```
### Usage

You can manage the kalertd daemon using standard systemctl commands:
```bash
sudo systemctl enable kalertd       # enable at boot
sudo systemctl disable kalertd      # disable at boot
sudo systemctl start kalertd        # start daemon
sudo systemctl stop kalertd         # stop daemon
sudo systemctl status kalertd       # check status
sudo systemctl reload kalertd       # reload configuration
```

## 3.libkalert
libkalert is a user-space library that wraps the low-level Netlink
protocol details of kalert. It provides simple interfaces for applications
to subscribe to kalert channels, receive kernel fault events, and
configure channel parameters, allowing developers to focus on processing
events without handling the Netlink protocol directly.

### Usage in your application

```c
#include <libkalert/libkalert.h>

int main() {
    kalert_msg(LOG_INFO, "Hello from my app");
    return 0;
}
```

### Compile your application using pkg-config

```bash
gcc your_app.c $(pkg-config --cflags --libs libkalert) -o your_app
```

## 4.Project Structure

```
${PROJECT_NAME}/
├── include/ # public headers
├── libkalert/ # library source code
├── src/ # daemon and example applications
├── build/ # build artifacts
└── Makefile
```
