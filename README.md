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
