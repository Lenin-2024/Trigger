#include <stdio.h>
#include <stdlib.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "SimplePoll"
#define TOPIC       "door"
#define QOS         0

int main(int argc, char **argv) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message *message = NULL;
    char *topic = NULL;
    int topicLen;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Ошибка подключения: %d\n", rc);
        return -1;
    }

    MQTTClient_subscribe(client, TOPIC, QOS); // подписка на топик
    printf("Слушаю. Нажмите Ctrl+C для выхода...\n");

    while(1) {
        rc = MQTTClient_receive(client, &topic, &topicLen, &message, 1000); // ожидаем сообщение
        
        if (rc == MQTTCLIENT_SUCCESS && message != NULL) {
            if (message->payloadlen == 1) {
                char value = ((char*)message->payload)[0];
                printf("Получено: %c\n", value);
            }
            
            MQTTClient_freeMessage(&message);
            MQTTClient_free(topic);
        }
    }

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    
    return 0;
}