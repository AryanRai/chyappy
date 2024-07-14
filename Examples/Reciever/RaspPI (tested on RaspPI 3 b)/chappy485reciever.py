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
