
---

# **chYAPpy v1.2**
### **Communication Protocol and Implementation for STM32, Arduino, and Raspberry Pi**

#### **Overview**
chYAPpy v1.2 is an enhanced custom protocol designed for transmitting sensor data between microcontrollers (e.g., STM32, Arduino/Teensy) and other devices (e.g., Raspberry Pi) over various communication mediums such as RS485, LoRa, and UART. It introduces versioning via start markers, adds sequence numbers for packet tracking, supports typed payloads for efficiency, and upgrades error detection with CRC-8. This protocol is optimized for reliability in noisy environments like high-performance rocketry while remaining backward-compatible with v1.1 through distinct start markers.

#### **Protocol Structure**
The message structure for chYAPpy v1.2 is as follows:

| Field              | Size (Bytes) | Description                                                                 |
|--------------------|--------------|-----------------------------------------------------------------------------|
| **Start Marker**   | 1            | Unique byte indicating the protocol version and message start (0x7D for v1.2). |
| **Length**         | 1            | Length of the payload (0-255 bytes).                                       |
| **Sensor Type**    | 1            | Character representing the sensor type (e.g., 'T' for temperature).        |
| **Sensor ID**      | 1            | Numeric identifier for the sensor (0-255).                                 |
| **Sequence Number**| 2            | 16-bit number for packet ordering and tracking (0-65535).                  |
| **Payload Type**   | 1            | Indicates the data type of the payload (e.g., 0x01 for string, 0x02 for float). |
| **Payload**        | N            | Variable-length sensor data, format depends on Payload Type.               |
| **Checksum**       | 1            | CRC-8 checksum for error detection (covers Length to Payload).             |

**Total Size:** `8 + N` bytes, where `N` is the payload length.

#### **Field Details**
1. **Start Marker (1 byte):**
   - Value: `0x7D` (distinguishes v1.2 from v1.1’s `0x7E`).
   - Purpose: Marks the beginning of the message and identifies the protocol version.

2. **Length (1 byte):**
   - Range: 0-255.
   - Purpose: Specifies the number of bytes in the payload (excludes header and checksum).

3. **Sensor Type (1 byte):**
   - Examples: `'T'` (temperature), `'A'` (accelerometer), `'G'` (gyroscope).
   - Purpose: Identifies the type of sensor data being transmitted.

4. **Sensor ID (1 byte):**
   - Range: 0-255.
   - Purpose: Uniquely identifies the sensor within its type (e.g., Thermocouple 1 vs. 2).

5. **Sequence Number (2 bytes):**
   - Range: 0-65535 (16-bit unsigned integer, big-endian).
   - Purpose: Tracks packet order and detects missing messages; increments per transmission.

6. **Payload Type (1 byte):**
   - Values:
     - `0x01`: String (ASCII-encoded, variable length).
     - `0x02`: Float (4 bytes, IEEE 754 single precision).
     - `0x03`: Int16 (2 bytes, signed).
     - `0x04`: Int32 (4 bytes, signed).
   - Purpose: Specifies the format of the payload for efficient parsing.

7. **Payload (N bytes):**
   - Content: Sensor data, formatted according to Payload Type.
   - Examples:
     - String: `"25.3662"` (7 bytes).
     - Float: 4-byte binary representation of 25.3662.
   - Purpose: Carries the actual sensor measurement.

8. **Checksum (1 byte):**
   - Algorithm: CRC-8 (Dallas/Maxim, polynomial 0x31).
   - Scope: Calculated over `Length`, `Sensor Type`, `Sensor ID`, `Sequence Number`, `Payload Type`, and `Payload`.
   - Purpose: Ensures data integrity with stronger error detection than v1.1’s XOR.

#### **Comparison with v1.1**
| Feature            | v1.1                     | v1.2                         |
|--------------------|--------------------------|------------------------------|
| **Start Marker**   | 0x7E (fixed)             | 0x7D (version-specific)      |
| **Sequence Number**| None                     | 2 bytes                      |
| **Payload Type**   | Implicit (string)        | Explicit (1 byte)            |
| **Checksum**       | XOR (1 byte)             | CRC-8 (1 byte)               |
| **Size Overhead**  | 5 bytes                  | 8 bytes                      |

#### **Example Messages**
1. **Temperature (String):**
   - Data: 25.3662°C from Thermocouple 1, sequence number 42.
   - Format: `| 0x7D | 0x07 | 'T' | 0x01 | 0x00 0x2A | 0x01 | "25.3662" | CRC |`
   - Hex: `7D 07 54 01 00 2A 01 32 35 2E 33 36 36 32 [CRC]`

2. **Acceleration (Float):**
   - Data: 9.81 m/s² from Accelerometer 2, sequence number 43.
   - Format: `| 0x7D | 0x04 | 'A' | 0x02 | 0x00 0x2B | 0x02 | [9.81 float] | CRC |`
   - Hex: `7D 04 41 02 00 2B 02 41 1C 7A E1 [CRC]`

#### **Implementation Notes**
- **Sender:**
  - Increment `Sequence Number` for each message.
  - Choose `Payload Type` based on data (e.g., string for human-readable logs, float for efficiency).
  - Compute CRC-8 over all bytes from `Length` to end of `Payload`.

- **Receiver:**
  - Check `Start Marker` to determine version (0x7D for v1.2, 0x7E for v1.1).
  - Validate `Length` against received bytes.
  - Verify CRC-8; discard if mismatched.
  - Parse `Payload` based on `Payload Type`.

- **Error Handling:**
  - If CRC fails, log the error and discard the packet.
  - Use `Sequence Number` to detect dropped packets.

#### **Reference Code (Arduino/Teensy)**
```cpp
#include <Arduino.h>

#define CHYAPPY_V1_2_START 0x7D
#define PAYLOAD_TYPE_STRING 0x01
#define PAYLOAD_TYPE_FLOAT 0x02

uint8_t crc8(const uint8_t* data, uint8_t len) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) crc = (crc << 1) ^ 0x31;
      else crc <<= 1;
    }
  }
  return crc;
}

void sendChYAPpyV12(Stream& output, uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, uint8_t payloadType, const uint8_t* payload, uint8_t length) {
  uint8_t message[length + 8];
  message[0] = CHYAPPY_V1_2_START;
  message[1] = length;
  message[2] = sensorType;
  message[3] = sensorId;
  message[4] = seqNum >> 8;
  message[5] = seqNum & 0xFF;
  message[6] = payloadType;
  for (uint8_t i = 0; i < length; i++) {
    message[7 + i] = payload[i];
  }
  message[length + 7] = crc8(&message[1], length + 6);

  output.write(message, length + 8);
  output.flush();
}

// Example usage
void sendTempExample(Stream& output) {
  float temp = 25.3662;
  uint16_t seq = 42;
  sendChYAPpyV12(output, 'T', 1, seq, PAYLOAD_TYPE_FLOAT, (uint8_t*)&temp, 4);
}
```

#### **Applications**
- **High-Performance Rocketry:** Reliable telemetry for sensors (temperature, IMU, ADC) over RS485 and LoRa.
- **CubeSats:** Efficient data transmission with type support for binary data.
- **Ground Stations:** Sequence numbers aid in reconstructing flight data timelines.

#### **Future Extensions (v2.0, Start Marker 0x7C)**
- Add flags for priority or acknowledgment.
- Support larger payloads with 2-byte length.
- Include an end marker for additional framing robustness.

---

# CHYappy v1.1

# V1.0
# Communication Protocol and Implementation for STM32, Arduino, and Raspberry Pi

## Overview
This documentation describes a custom protocol for sending sensor data from an STM32 microcontroller to an Arduino and a Raspberry Pi using RS485 and UART. The protocol includes the sensor type, sensor identifier, and sensor data, with a checksum for data integrity.

## Protocol Structure
The message structure is defined as follows:
```
| Start Marker (1 byte) | Length (1 byte) | Sensor Type (1 byte) | Sensor ID (1 byte) | Payload (N bytes) | Checksum (1 byte) |
```

- **Start Marker**: A unique byte (0x7E) indicating the start of the message.
- **Length**: The length of the payload.
- **Sensor Type**: A character representing the sensor type (e.g., 'T' for temperature).
- **Sensor ID**: A number representing the sensor identifier.
- **Payload**: The actual sensor data (e.g., a string representation of a float).
- **Checksum**: A checksum byte for error detection (XOR of all payload bytes).


## Hardware Setup
I am using RS485 to TTL converter which has the MAX485 chip in it. 

### Sender
I am using USART1 for RS485 and USART2 for debugging 

- DI - RX pin of USART
- RO - TX pin of USART
- DE - connected to RE - Pin PA9  
- RE - connected to DE - Pin PA9 
 
### Receiver
ESP32
- DI - RX pin of USART - Pin 18
- RO - TX pin of USART - Pin 19 
- DE - connected to RE - Pin 4  
- RE - connected to DE - Pin 4 

## STM32 Sender Code

### STM32 Sender Code (Example) (Check actual repo for updated one)
```c
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define START_MARKER 0x7E

void send_message(uint8_t sensor_type, uint8_t sensor_id, uint8_t *payload, uint8_t length) {
    uint8_t message[length + 5];
    uint8_t checksum = 0;

    message[0] = START_MARKER;
    message[1] = length;
    message[2] = sensor_type;
    message[3] = sensor_id;

    for (uint8_t i = 0; i < length; i++) {
        message[4 + i] = payload[i];
        checksum ^= payload[i];
    }

    message[length + 4] = checksum;

    HAL_GPIO_WritePin(TX_EN_GPIO_Port, TX_EN_Pin,  1);
	HAL_UART_Transmit(&huart1,  message,  length  +  5, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(TX_EN_GPIO_Port, TX_EN_Pin,  0);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();

    while (1) {
        // Main loop
        uint8_t sensor_type = 'T';
	    uint8_t sensor_id = 1;
	    float sensor_value = 25.3662;
	    char payload[10];
	    
	    snprintf(payload, sizeof(payload), "%.4f", sensor_value);
	    uint8_t length = strlen(payload);

	    send_message(sensor_type, sensor_id, (uint8_t *)payload, length);
    }
}
```

### Explanation
- **HAL_Init, SystemClock_Config, and MX_USART1_UART_Init**: Initialize the HAL library, configure the system clock, and initialize the UART peripheral.
- **send_message Function**: Constructs and sends the message according to the defined protocol.
- **Main Function**: Sets up the system, constructs a message with sensor type 'T', sensor ID 1, and payload `25.3662`, then sends the message.

## Arduino Receiver Code

### Arduino Receiver Code (Example)
```cpp
  

```

### Explanation
- **START_MARKER**: Defines the start of the message.
- **Serial.available and Serial.read**: Check and read bytes from the serial buffer.
- **Checksum Calculation**: Validate the checksum for data integrity.
- **process_message Function**: Converts the payload to a string and prints the sensor type, sensor ID, and payload value.


### Raspberry Pi Receiver Code (Python)

```python
import serial

START_MARKER = 0x7E

def calculate_checksum(data):
    """Calculate the XOR checksum of the given data."""
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def process_message(sensor_type, sensor_id, payload):
    """Process the received message and print the details."""
    # Print the sensor type, sensor ID, and payload in hexadecimal format
    print(f"Received sensor type: {chr(sensor_type)}")
    print(f"Sensor ID: {sensor_id}")
    print("Payload (hex):", ' '.join(format(x, '02x') for x in payload))

    # Convert the payload to a string and print
    payload_str = ''.join(chr(x) for x in payload)
    print("Payload (as string):", payload_str)

def main():
    # Set up the serial connection (adjust the port and baud rate as needed)
    ser = serial.Serial('/dev/ttyS0', 115200, timeout=1)
    
    while True:
        if ser.in_waiting > 0:
            # Read the start marker
            if ser.read() == bytes([START_MARKER]):
                # Read the length, sensor type, and sensor ID
                length = ser.read()[0]
                sensor_type = ser.read()[0]
                sensor_id = ser.read()[0]
                
                # Read the payload and checksum
                payload = ser.read(length)
                received_checksum = ser.read()[0]
                
                # Verify the checksum
                if calculate_checksum(payload) == received_checksum:
                    process_message(sensor_type, sensor_id, payload)
                else:
                    print("Checksum error")

if __name__ == "__main__":
    main()
```

### Explanation
- **START_MARKER**: The unique byte (0x7E) indicating the start of a message.
- **calculate_checksum**: Function to calculate the XOR checksum for the given data.
- **process_message**: Function to process the received message. It prints the sensor type, sensor ID, and payload (both as hexadecimal values and as a string).
- **main**: The main function sets up the serial connection and continuously reads incoming messages. It reads the start marker, length, sensor type, sensor ID, payload, and checksum, and then verifies the checksum before processing the message.

### Configuration
- **Serial Port**: Adjust `/dev/ttyS0` to match the serial port you are using on the Raspberry Pi. For example, it might be `/dev/ttyUSB0` if you are using a USB-to-RS485 adapter.
- **Baud Rate**: Ensure the baud rate matches the one set on the STM32 and Arduino.

### Usage
1. **Connect the RS485 transceivers** to the STM32, Arduino, and Raspberry Pi.
2. **Run the Python script** on the Raspberry Pi.
3. **Send a message** from the STM32 using the provided STM32 sender code.
4. **Observe the output** on the Raspberry Pi terminal, which should display the received message details.

## Configuration
- **Baud Rate**: Ensure the same baud rate is set for STM32, Arduino, and Raspberry Pi.
- **Serial Ports**: Adjust the serial port names in the code as needed (`/dev/ttyS0` for Raspberry Pi, `Serial` for Arduino).

## Usage
1. **STM32**: Initialize the system, set up the UART, construct the message, and send it.
2. **Arduino**: Set up the serial connection, read and validate incoming messages, and process the payload.
3. **Raspberry Pi**: Set up the serial connection using `pyserial`, read and validate incoming messages, and process the payload.

This documentation provides a complete guide to setting up and using 'chyappy' the protocol for communication between STM32, Arduino, and Raspberry Pi over RS485 using UART. 
