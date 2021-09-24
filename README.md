# Codec Server

The Codec Server will coordinate central dispatching and coordination of (hardware or software) codecs over the network.

The network stack is connection-oriented, and runs on Unix domain sockets or TCP over IPv4 or IPv6.

It uses a protocol based on [protocol buffers](https://developers.google.com/protocol-buffers) (or protobuf for short) to communicate, exchanging length-delimited Any-encapsulated messages on the sockets.

The codec system is modular, and can be extended with custom drivers and modules. The base package comes with the `ambe3k` driver to interface with AMBE-3000 devices over a serial interface.

The primary use case for this application is to allow decoding of AMBE-based digital voice modes in the [OpenWebRX](https://www.openwebrx.de) project, but the basic design has been kept neutral so it can be used in other applications as well.

# Installation

The OpenWebRX project is hosting codecserver packages in their repositories. Please click the respective link for [Debian](https://www.openwebrx.de/download/debian.php) or [Ubuntu](https://www.openwebrx.de/download/ubuntu.php). 

# Compiling from sources

Before compiling, please make sure you have the necessary protobuf libraries installed. Most distributions will ship separate development packages for the development files.

For example, on Debian distributions you will need to install these packages:

```
apt-get install libprotobuf-dev protobuf-compiler
```

This project comes with a cmake build. It is recommended to build in a separate directory.

```
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```

A sample configuration is available in the `conf` directory. It needs to be installed manually before the program can run.

# Configuration

The configuration file is `/etc/codecserver/codecserver.conf` or `/usr/local/etc/codecserver/codecserver.conf`, depending on your installation.

The config file uses an INI-style syntax. The section headers use prefixes to specify what part of the system the configuration applies to:

* `[server:{type}]` sections specify the connectivity that codecserver will make available. The available types are `tcp`, `tcp4` and `unixdomainsockets`. Only the specified servers will be started.
* `[driver:{id}]` sections can be used to specify options that apply to the driver, or all devices of that driver. `id` must match the identification of the driver when registering. Fields of these sections are driver-specific.
* `[device:{name}]` sections allow you to add manually-configured devices. `name` can be freely selected, but must be unique within your configuration. Sections of this kind must have a `type={id}` declaration to specify what driver this config should use. Other fields are driver-specific.

# systemd integration

Since most current linux distributions have adopted systemd by now, codecserver will try to register itself as a systemd service during installation. When manually compiling, you will need to run `systemctl daemon-reload` on initial installation. After that, you should be able to control the codecserver service just like any other service unit.

Examples:

```
systemctl start codecserver
systemctl stop codecserver
systemctl restart codecserver
systemctl enable codecserver
systemctl disable codecserver
journalctl -u codecserver
```

# Docker

You can find [codecserver Docker images on the Docker hub](https://hub.docker.com/r/jketterl/codecserver). Please have a look at [the wiki](https://github.com/jketterl/codecserver/wiki/Docker) for more information.