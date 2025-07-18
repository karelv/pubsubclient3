/*
 Example of using a Stream object to store the message payload

 Uses SRAM library: https://github.com/ennui2342/arduino-sram
 but could use any Stream based class such as SD

  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <SRAM.h>

// Update these with values suitable for your network.
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);

SRAM sram(4, SRAM_1024);

void callback(char* topic, uint8_t* payload, size_t plength) {
    sram.seek(1);

    // do something with the message
    for (size_t i = 0; i < plength; i++) {
        Serial.write(sram.read());
    }
    Serial.println();

    // Reset position for the next message to be stored
    sram.seek(1);
}

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient, sram);

void setup() {
    Ethernet.begin(mac, ip);
    if (client.connect("arduinoClient")) {
        client.publish("outTopic", "hello world");
        client.subscribe("inTopic");
    }

    sram.begin();
    sram.seek(1);

    Serial.begin(9600);
}

void loop() {
    client.loop();
}
