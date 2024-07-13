# chyappy
a data simple easy to use one way sensor data transmission protocol for RS485 and other applications specifically when logging  

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

## Raspberry Pi Receiver Code (Python)

### Raspberry Pi Receiver Code (Example)
```python

```

### Explanation
- **START_MARKER**: Defines the start of the message.
- **calculate_checksum**: Function to calculate the checksum for validation.
- **process_message Function**: Prints the sensor type, sensor ID, and payload. Converts the payload to a string and prints it.
- **main Function**: Reads and processes incoming messages from the serial port.

## Configuration
- **Baud Rate**: Ensure the same baud rate is set for STM32, Arduino, and Raspberry Pi.
- **Serial Ports**: Adjust the serial port names in the code as needed (`/dev/ttyS0` for Raspberry Pi, `Serial` for Arduino).

## Usage
1. **STM32**: Initialize the system, set up the UART, construct the message, and send it.
2. **Arduino**: Set up the serial connection, read and validate incoming messages, and process the payload.
3. **Raspberry Pi**: Set up the serial connection using `pyserial`, read and validate incoming messages, and process the payload.

This documentation provides a complete guide to setting up and using 'chyappy' the protocol for communication between STM32, Arduino, and Raspberry Pi over RS485 using UART. 
