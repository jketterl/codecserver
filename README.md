# Codec Server

The Codec Server will coordinate central dispatching and coordination of (hardware or software) codecs over the network.

The network stack is based is connection-oriented, and runs on Unix domain sockets or TCP over IPv4 or IPv6.

It uses a protocol based on [protocol buffers](https://developers.google.com/protocol-buffers) (or protobuf for short) to communicate, exchanging length-delimited Any-encapsulated messages on the sockets.

The codec system is modular, and can be extended with custom drivers and modules. The base package comes with the `ambe3k` driver to interface with AMBE-3000 devices over a serial interface.

The primary use case for this application is to allow decoding of AMBE-based digital voice modes in the [OpenWebRX](https://www.openwebrx.de) project, but the basic design has been kept neutral so it can be used in other applications as well. 