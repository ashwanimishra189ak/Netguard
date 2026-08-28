# NetGuard

NetGuard is a C++ based Deep Packet Inspection (DPI) engine for analyzing PCAP network traffic, identifying applications using TLS SNI, tracking network flows, and applying configurable blocking rules.

## Features

* PCAP file reading and packet processing
* Ethernet, IPv4, TCP and UDP packet parsing
* TLS Client Hello SNI extraction
* Application identification from domains
* Flow-based traffic tracking using Five-Tuple
* IP, application and domain blocking
* Single-threaded DPI processing
* Multi-threaded processing architecture
* Packet filtering and output PCAP generation
* Application and packet statistics

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
```

## Project Structure

```text
NetGuard/
├── include/
├── src/
├── CMakeLists.txt
├── README.md
├── WINDOWS_SETUP.md
├── generate_test_pcap.py
└── .gitignore
```

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

```bash
./dpi_engine <input.pcap> <output.pcap>
```

### Block Application

```bash
./dpi_engine ../test_dpi.pcap ../filtered_blocked.pcap \
--block-app YouTube
```

### Block Domain

```bash
./dpi_engine ../test_dpi.pcap ../filtered_blocked.pcap \
--block-domain tiktok
```

### Block Source IP

```bash
./dpi_engine ../test_dpi.pcap ../filtered_ip_blocked.pcap \
--block-ip 192.168.1.100
```

## Testing

NetGuard was tested using a sample PCAP containing traffic for multiple applications and domains.

### Application and Domain Blocking

```text
Total Packets:       77
Forwarded:           75
Dropped/Blocked:      2
Drop Rate:           2.60%
```

Detected blocked traffic:

```text
BLOCKED packet: App YouTube
BLOCKED packet: Domain www.tiktok.com
```

### IP Blocking

```text
Total Packets:       77
Forwarded:           21
Dropped/Blocked:     56
Drop Rate:           72.73%
```

Blocked rule:

```text
Blocked IP: 192.168.1.100
```

### TLS SNI Detection

TLS SNI extraction was verified using TShark. The test PCAP successfully exposed domains including:

```text
www.google.com
www.facebook.com
github.com
www.instagram.com
zoom.us
twitter.com
www.amazon.com
www.netflix.com
discord.com
web.telegram.org
www.tiktok.com
open.spotify.com
www.microsoft.com
www.apple.com
```

## Test Statistics

| Metric                   | Result |
| ------------------------ | -----: |
| Total Packets            |     77 |
| Total Bytes              |   5738 |
| TCP Packets              |     73 |
| UDP Packets              |      4 |
| Total Connections        |     43 |
| Classified Connections   |     22 |
| Unidentified Connections |     21 |

### Multi-threaded Configuration

```text
Load Balancers:          2
Fast Path Processors:    4
FPs per Load Balancer:   2
```

## Limitations

* Application classification depends on available domain/SNI information.
* Encrypted traffic without visible SNI may remain unidentified.
* Current implementation primarily supports PCAP-based analysis.

## Future Improvements

* Live network interface capture
* Additional application signatures
* Improved encrypted traffic classification
* Real-time monitoring dashboard
* Extended traffic analytics

## License

This project is intended for educational and development purposes.
