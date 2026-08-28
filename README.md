# NetGuard

NetGuard is a C++ based Deep Packet Inspection (DPI) engine for analyzing PCAP network traffic, identifying applications using TLS SNI, tracking network flows, and applying configurable blocking rules.

## Features

- PCAP file reading and packet processing
- Ethernet, IPv4, TCP and UDP packet parsing
- TLS Client Hello SNI extraction
- Application identification from domains
- Flow-based traffic tracking using Five-Tuple
- IP, application and domain blocking
- Single-threaded DPI processing
- Multi-threaded processing architecture
- Packet filtering and output PCAP generation
- Application and packet statistics

## Architecture

NetGuard processes packets through the following pipeline:

```text
PCAP Input
    |
    v
PCAP Reader
    |
    v
Packet Parser
    |
    v
Five-Tuple / Flow Tracking
    |
    v
DPI / SNI Extraction
    |
    v
Application Classification
    |
    v
Blocking Rules
    |
    +-------- BLOCK --------> Drop Packet
    |
    +-------- ALLOW --------> Output PCAP
