#include "BDDTest.h"
#include "Buffer.h"
#include "PubSubClient.h"
#include "ShimClient.h"
#include "trace.h"

byte server[] = {172, 16, 0, 2};

// function declarations
void callback(char* topic, uint8_t* payload, size_t plength);
int test_subscribe_no_qos();
int test_subscribe_qos_1();
int test_subscribe_not_connected();
int test_subscribe_invalid_qos();
int test_subscribe_too_long();
int test_unsubscribe();
int test_unsubscribe_not_connected();

void callback(_UNUSED_ char* topic, _UNUSED_ uint8_t* payload, _UNUSED_ size_t plength) {
    // handle message arrived
}

int test_subscribe_no_qos() {
    IT("subscribe without QoS (defaults to 0)");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte subscribe[] = {0x82, 0xa, 0x0, 0x2, 0x0, 0x5, 0x74, 0x6f, 0x70, 0x69, 0x63, 0x0};
    shimClient.expect(subscribe, 12);
    byte suback[] = {0x90, 0x3, 0x0, 0x2, 0x0};
    shimClient.respond(suback, 5);

    rc = client.subscribe("topic");
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_subscribe_qos_1() {
    IT("subscribe with QoS 1");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte subscribe[] = {0x82, 0xa, 0x0, 0x2, 0x0, 0x5, 0x74, 0x6f, 0x70, 0x69, 0x63, 0x1};
    shimClient.expect(subscribe, 12);
    byte suback[] = {0x90, 0x3, 0x0, 0x2, 0x1};
    shimClient.respond(suback, 5);

    rc = client.subscribe("topic", MQTT_QOS1);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_subscribe_not_connected() {
    IT("subscribe fails when not connected");
    ShimClient shimClient;

    PubSubClient client(server, 1883, callback, shimClient);

    bool rc = client.subscribe("topic");
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_subscribe_invalid_qos() {
    IT("subscribe fails with invalid QoS values");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    rc = client.subscribe("topic", MQTT_QOS2);
    IS_FALSE(rc);
    rc = client.subscribe("topic", 254);
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_subscribe_too_long() {
    IT("subscribe fails with too long topic");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    client.setBufferSize(128);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    // max length should be allowed (buffer size - MQTT_MAX_HEADER_SIZE - msgId (2) - topic length bytes (2) - QoS (1) = 118)
    //                     0        1         2         3         4         5         6         7         8         9         0         1         2
    rc = client.subscribe("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678");
    IS_TRUE(rc);

    //                     0        1         2         3         4         5         6         7         8         9         0         1         2
    rc = client.subscribe("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_unsubscribe() {
    IT("unsubscribes");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte unsubscribe[] = {0xA2, 0x9, 0x0, 0x2, 0x0, 0x5, 0x74, 0x6f, 0x70, 0x69, 0x63};
    shimClient.expect(unsubscribe, 12);
    byte unsuback[] = {0xB0, 0x2, 0x0, 0x2};
    shimClient.respond(unsuback, 4);

    rc = client.unsubscribe("topic");
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_unsubscribe_not_connected() {
    IT("unsubscribe fails when not connected");
    ShimClient shimClient;

    PubSubClient client(server, 1883, callback, shimClient);

    bool rc = client.unsubscribe("topic");
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int main() {
    SUITE("Subscribe");
    test_subscribe_no_qos();
    test_subscribe_qos_1();
    test_subscribe_not_connected();
    test_subscribe_invalid_qos();
    test_subscribe_too_long();
    test_unsubscribe();
    test_unsubscribe_not_connected();
    FINISH
}
