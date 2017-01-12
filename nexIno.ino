#include "NexaCtrl.h"

#define TX_PIN 13
#define RX_PIN 8

NexaCtrl nexaCtrl(TX_PIN, RX_PIN);

const static unsigned long controller_id = 20677082;
const uint8_t num_of_input_characters = 3; // Number of expected input arguments

unsigned int device = 0;
unsigned int dim_level = 0;
bool sequence_read = false;
String delimiter = "/";
String inData;
int c_incoming_num[num_of_input_characters] {0}; // Array: [device_id, mode, dim_level]
uint8_t index = 0;
char received;

//ASCII-codes
const char ascii_eol = 0x25;        // '%'
const char ascii_delimiter = 0x2F;  // '/'
const char ascii_zero = 0x30;       // '0'
const char ascii_nine = 0x39;       // '9'

void setup() {
    Serial.begin(9600);
    Serial.println("Setup complete!");
    // Set all elements to 99 (= default value)
    for(int i=0; i<num_of_input_characters; i++) {c_incoming_num[i] = 99;}
}

void loop() {
    /*
    * Listen for incoming data
    */
    if (Serial.available() > 0) {
        
        /*
        * Read data over Serial
        */
        while (Serial.available() > 0) {
            
            received = Serial.read(); // One charachter at a time
            if ( received != ascii_eol ) { // EOL not reached
                if ( received == ascii_delimiter ) { // Got delimiter between input data
                    c_incoming_num[index] = inData.toInt();
                    inData = "";
                    index++;
                } else {
                    if ( received >= ascii_zero && received <= ascii_nine ) {    // Accept only integers
                        inData += received;
                    }
                }
            } else { // EOL reached
                if ( inData != "" ) {
                    c_incoming_num[index] = inData.toInt();
                    inData = "";
                }
                index = 0;
                sequence_read = true;
                break;
            }
        }

        /*
        * Send control command over 433 MHz
        */
        if (sequence_read) {
            device = c_incoming_num[0]-1;   // Nexa device 1 uses device_id 0 etc.
            dim_level = c_incoming_num[2];
            
            if (dim_level <= 15) { // A dim level was specified. Dim level in [0..15]
                nexaCtrl.DeviceDim(controller_id, device, dim_level);
            } else {
                switch (c_incoming_num[1]) {  // Check specified mode
                    case 0:
                        nexaCtrl.DeviceOff(controller_id, device);
                        break;
                    case 1:
                        nexaCtrl.DeviceOn(controller_id, device);
                        break;
                    default:
                        break;
                }
            }
            
            // Reset and get ready for next transfer
            for(int i=0; i<num_of_input_characters; i++) {c_incoming_num[i] = 99;}
            sequence_read = false;
        }
    }
//
}
