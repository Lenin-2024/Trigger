#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.3.1:1883"
#define CLIENTID    "SimplePub"
#define TOPIC       "door"
#define QOS         0

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
    int value = 0;

    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Ошибка подключения: %d\n", rc);
        return -1;
    }

    printf("Подключен! Отправляю 1 и 0...\n");

    for(int i = 0; i < 10; i++) {
        value = i % 2;

        char payload[2];
        payload[0] = value + '0';
        payload[1] = '\0';

        pubmsg.payload = payload;   // само сообщение
        pubmsg.payloadlen = 1;      // длинна
        pubmsg.qos = 0;             // qos 0 - без подтверждения
        pubmsg.retained = 0;        // не сохранять на брокере

        MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token); // отправка
        printf("Отправлено: %d\n", value);

        sleep(1);
    }

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);

    return 0;
}
