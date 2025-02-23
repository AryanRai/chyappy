
---

# **chYAPpy Protocol Changelog**

All notable changes to the chYAPpy communication protocol will be documented in this file.

## **[1.2] - 2025-02-22**
### **Added**
- **Versioned Start Markers:** Introduced distinct start markers to differentiate protocol versions:
  - `0x7D` for v1.2 (new).
  - Retained `0x7E` for v1.1 compatibility.
- **Sequence Number:** Added a 2-byte field (16-bit, 0-65535) after `Sensor ID` for packet ordering and loss detection.
- **Payload Type:** Added a 1-byte field after `Sequence Number` to specify data type:
  - `0x01`: String (ASCII, variable length).
  - `0x02`: Float (4 bytes, IEEE 754).
  - `0x03`: Int16 (2 bytes, signed).
  - `0x04`: Int32 (4 bytes, signed).
- **Documentation:** Comprehensive v1.2 documentation added, including examples and implementation notes.

### **Changed**
- **Checksum:** Replaced 1-byte XOR checksum with CRC-8 (Dallas/Maxim, polynomial 0x31) for stronger error detection.
  - CRC-8 now covers `Length` through `Payload` (excluding `Start Marker`).
- **Message Structure:** Updated to `| Start (1) | Length (1) | Sensor Type (1) | Sensor ID (1) | SeqNum (2) | Payload Type (1) | Payload (N) | CRC-8 (1) |`.
  - Overhead increased from 5 to 8 bytes.

### **Notes**
- Backward compatibility maintained with v1.1 via `0x7E` start marker; receivers must check the start byte to parse correctly.
- Designed for enhanced reliability in high-performance rocketry applications (e.g., BlueLily).

---

## **[1.1] - 2024-10-XX]**
*(Assumed release date prior to v1.2; adjust as needed)*

### **Added**
- **Initial Documentation:** Formalized protocol structure in documentation for STM32, Arduino, and Raspberry Pi use.

### **Changed**
- **Length Field:** Clarified that `Length` represents payload size only (excluding header/checksum), consistent with v1.0 intent.
- **Checksum Scope:** Explicitly defined XOR checksum as calculated over payload bytes only.

### **Notes**
- Minor refinement of v1.0; no structural changes to message format.
- Start marker remained `0x7E`.

---

## **[1.0] - 2024-7-XX]**
*(Assumed initial release; adjust as needed)*

### **Added**
- **Initial Protocol:** Introduced chYAPpy as a custom protocol for sensor data transmission over RS485 and UART.
- **Message Structure:** Defined as `| Start (1) | Length (1) | Sensor Type (1) | Sensor ID (1) | Payload (N) | Checksum (1) |`.
  - Start Marker: `0x7E`.
  - Length: Payload size (1 byte, 0-255).
  - Sensor Type: 1-byte character (e.g., 'T').
  - Sensor ID: 1-byte identifier (0-255).
  - Payload: Variable-length string data.
  - Checksum: 1-byte XOR of payload bytes.

### **Notes**
- Baseline version designed for simplicity and basic error detection.
- First implemented for STM32-to-Arduino/Raspberry Pi communication.

---

### **Versioning Notes**
- Versions follow a `[major].[minor]` format.
- Major versions (e.g., 2.0) may introduce breaking changes and use a new start marker (e.g., `0x7C` reserved for v2.0).
- Minor versions (e.g., 1.1, 1.2) add features while maintaining compatibility with prior minor versions within the same major branch.

### **Contributing**
Suggestions for improvements or bug reports can be discussed with the BlueLily development team (buzzaryanrai@gmail.com).

---
