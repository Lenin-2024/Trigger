#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
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
    while (1) {
        int found = 0;
        struct stat st;
        if (stat("file_to_create", &st) == 0) {
            printf("maybe");
            if (S_ISREG(st.st_mode)){
                found = 1;
                break;
            }
        }

        if(found){
            char buf[255];
            sprintf(buf, "%d %d", 0, 1);
            pubmsg.payload = buf;   // само сообщение
            pubmsg.payloadlen = strlen(buf);    // длина
            pubmsg.qos = 0;             // qos 0 - без подтверждения
            pubmsg.retained = 0;        // не сохранять на брокере
            MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token); // отправка
            printf("\nTask completed:\n", value);
            break;
        }

        sleep(1);
    }

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);

    return 2;
}
