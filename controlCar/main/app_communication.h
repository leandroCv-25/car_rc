#ifndef APP_COMMUNICATION_H_
#define APP_COMMUNICATION_H_

// Callback typedef
typedef void (*app_communication_callback_t)(bool isBroadcast, uint8_t *srcAddress, const uint8_t *data, int len);

typedef struct communication_app_msg_t
{
    uint8_t destAddress[6];
    uint8_t data[250];
}
communication_app_msg_t;

BaseType_t appCommunicationEvent(uint8_t *adrDst);
BaseType_t appCommunicationEventWithData(uint8_t *data, uint8_t *adrDst);

BaseType_t appCommunicationBroadcastEvent();
BaseType_t appCommunicationBroadcastEventWithData(uint8_t *data);

void AddPeerAppCommunication(uint8_t *mac);
void DelPeerAppCommunication(uint8_t *mac);

void configAppCommunication(app_communication_callback_t responseCb);

#endif