import serial

START_MARKER = 0x7E

def calculate_checksum(data):
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def process_message(sensor_type, sensor_id, payload):
    # Implement sensor-specific logic here
    print(f"Received sensor type: {chr(sensor_type)}")
    print(f"Sensor ID: {sensor_id}")
    print("Payload:", ' '.join(format(x, '02x') for x in payload))

def main():
    ser = serial.Serial('/dev/ttyS0', 115200, timeout=1)  # Adjust the serial port and baud rate as needed

    while True:
        if ser.in_waiting > 0:
            if ser.read() == bytes([START_MARKER]):
                length = ser.read()[0]
                sensor_type = ser.read()[0]
                sensor_id = ser.read()[0]
                payload = ser.read(length)
                received_checksum = ser.read()[0]

                if calculate_checksum(payload) == received_checksum:
                    process_message(sensor_type, sensor_id, payload)
                else:
                    print("Checksum error")

if __name__ == "__main__":
    main()
