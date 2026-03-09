#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.3.1:1883"
#define CLIENTID    "SimplePub"
#define TOPIC       "door"
#define QOS         0

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "[ INFO ] Useage %s door number %s status %s\n", argv[0], "1..n", "1 or 0");
        return 1;
    }

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("[ ERROR ] Filed to connect: %d\n", rc);
        return -1;
    }

    char buf[255];
    sprintf(buf, "%s %s", argv[1], argv[2]);
    pubmsg.payload = buf;   // само сообщение
    pubmsg.payloadlen = strlen(buf);      // длинна
    pubmsg.qos = 0;             // qos 0 - без подтверждения
    pubmsg.retained = 0;        // не сохранять на брокере

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token); // отправка
    printf("[ INFO ] Send %s to door %s\n", argv[1], argv[2]);

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);

    return 0;
}
