
/***********************************************************************
 * Project      :     rs485 chYappy reciever 
 * Description  :     chYap over RS485            
 * Author       :     Aryan Rai

 ***********************************************************************/

#define START_MARKER 0x7E
/**************************************/
/*         include library            */
/**************************************/ 
#include <String.h>



/**************************************/
/*           GPIO define              */
/**************************************/
#define RS485_PIN_DIR 4
#define RXD2 18
#define TXD2 19




/**************************************/
/*        object define               */
/**************************************/





/**************************************/
/*        word define               */
/**************************************/
HardwareSerial rs485(1);

#define RS485_WRITE     1
#define RS485_READ      0

/**************************************/
/*        define function define      */
/**************************************/



/**************************************/
/*        global variable             */
/**************************************/


 
 /***********************************************************************
 * FUNCTION:    setup
 * DESCRIPTION: setup process
 * PARAMETERS:  nothing
 * RETURNED:    nothing
 ***********************************************************************/
void setup() {
  Serial.begin(115200);
  rs485.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.print("Info: Intial gpio...");
  pinMode(RS485_PIN_DIR,OUTPUT);
  digitalWrite(RS485_PIN_DIR,RS485_READ);
  Serial.println("done"); 

}

 /***********************************************************************
 * FUNCTION:    loop
 * DESCRIPTION: loop process
 * PARAMETERS:  nothing
 * RETURNED:    nothing
 ***********************************************************************/
 /***********************************************************************

chYappy v1 

| Start Marker (1 byte) | Length (1 byte) | Sensor Type (1 byte) | Sensor ID (1 byte) | Payload (N bytes) | Checksum (1 byte) |

 ***********************************************************************/

void process_message(uint8_t sensor_type, uint8_t sensor_id, uint8_t *payload, uint8_t length) {
    // Convert the payload to a string
    char payload_str[length + 1];
    memcpy(payload_str, payload, length);
    payload_str[length] = '\0';  // Null-terminate the string

    // Implement sensor-specific logic here
    Serial.print("Received sensor type: ");
    Serial.print((char)sensor_type);
    Serial.print(" ");
    Serial.print("Sensor ID: ");
    Serial.println(sensor_id);
    Serial.print("Payload: ");
    for (uint8_t i = 0; i < length; i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
    }
    Serial.print("Payload String: ");
    Serial.println(payload_str);
    Serial.println();
}

void loop() {


     digitalWrite(RS485_PIN_DIR,RS485_READ);
     
     if(rs485.available()){     //receive data from rs485   
      
          if (rs485.read() == START_MARKER) {
            while (rs485.available() < 3); // Wait for length, sensor type, and sensor id

            uint8_t length = rs485.read();
            uint8_t sensor_type = rs485.read();
            uint8_t sensor_id = rs485.read();
            uint8_t payload[length];
            uint8_t checksum = 0;

            for (uint8_t i = 0; i < length; i++) {
                while (rs485.available() == 0);
                payload[i] = rs485.read();
                checksum ^= payload[i];
            }

            while (rs485.available() == 0);
            uint8_t received_checksum = rs485.read();

            if (checksum == received_checksum) {
                // Process the payload with the sensor type and ID
                process_message(sensor_type, sensor_id, payload, length);
            } else {
                // Handle checksum error
                Serial.println("Checksum error");
            }
        }
 
      }   

  
}
