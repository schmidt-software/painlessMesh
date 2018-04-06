#include <Arduino.h>
#include <ArduinoJson.h>

#include "painlessMesh.h"
#include "painlessMeshSync.h"


#include "lwip/init.h"

painlessMesh* staticThis;
uint16_t  count = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
ICACHE_FLASH_ATTR painlessMesh::painlessMesh() {}
#pragma GCC diagnostic pop

// general functions
//***********************************************************************
void ICACHE_FLASH_ATTR painlessMesh::init(String ssid, String password, Scheduler *baseScheduler, uint16_t port, WiFiMode_t connectMode, wifi_auth_mode_t authmode, uint8_t channel, uint8_t phymode, uint8_t maxtpw, uint8_t hidden, uint8_t maxconn) {

    baseScheduler->setHighPriorityScheduler(&this->_scheduler);
    isExternalScheduler = true;

    init(ssid, password, port, connectMode, authmode, channel, phymode, maxtpw, hidden, maxconn);
}

void ICACHE_FLASH_ATTR painlessMesh::init(String ssid, String password, uint16_t port, WiFiMode_t connectMode, wifi_auth_mode_t authmode, uint8_t channel, uint8_t phymode, uint8_t maxtpw, uint8_t hidden, uint8_t maxconn) {
    // shut everything down, start with a blank slate.

    randomSeed(analogRead(A0)); // Init random generator seed to generate delay variance

#ifdef ESP32
    tcpip_adapter_init();
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    if ((esp_wifi_init(&init_config)) != ESP_OK) {
        //debugMsg(ERROR, "Station is doing something... wierd!? status=%d\n", err);
    }
#endif // ESP32

    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK)
        debugMsg(ERROR, "Unable to set storage to RAM only.\n");
        
    debugMsg(STARTUP, "init(): %d\n", WiFi.setAutoConnect(false)); // Disable autoconnect

    // Should check whether WIFI_AP etc.
    esp_wifi_set_protocol(ESP_IF_WIFI_STA, phymode);
    esp_wifi_set_protocol(ESP_IF_WIFI_AP, phymode);
#ifdef ESP8266
    system_phy_set_max_tpw(maxtpw); //maximum value of RF Tx Power, unit : 0.25dBm, range [0,82]
#endif
    esp_event_loop_init(espWifiEventCb, NULL);

    staticThis = this;  // provides a way for static callback methods to access "this" object;

    // start configuration
    switch (connectMode) {
    case WIFI_STA:
        debugMsg(GENERAL, "WiFi.mode(WIFI_STA) succeeded? %d\n", WiFi.mode(WIFI_STA));
        break;
    case WIFI_AP:
        debugMsg(GENERAL, "WiFi.mode(WIFI_AP) succeeded? %d\n", WiFi.mode(WIFI_AP));
        break;
    default:
        debugMsg(GENERAL, "WiFi.mode(WIFI_AP_STA) succeeded? %d\n", WiFi.mode(WIFI_AP_STA));
    }

    _meshSSID = ssid;
    _meshPassword = password;
    _meshPort = port;
    _meshChannel = channel;
    _meshAuthMode = authmode;
    if (password == "")
        _meshAuthMode = WIFI_AUTH_OPEN; //if no password ... set auth mode to open
    _meshHidden = hidden;
    _meshMaxConn = maxconn;

    uint8_t MAC[] = {0, 0, 0, 0, 0, 0};
    if (WiFi.softAPmacAddress(MAC) == 0) {
        debugMsg(ERROR, "init(): WiFi.softAPmacAddress(MAC) failed.\n");
    }
    esp_wifi_start();
    _nodeId = encodeNodeId(MAC);

    _apIp = IPAddress(0, 0, 0, 0);

    if (connectMode == WIFI_AP || connectMode == WIFI_AP_STA)
        apInit();       // setup AP
    if (connectMode == WIFI_STA || connectMode == WIFI_AP_STA) {
        stationScan.init(this, ssid, password, port);
        _scheduler.addTask(stationScan.task);
    }

    //debugMsg(STARTUP, "init(): tcp_max_con=%u, nodeId = %u\n", espconn_tcp_get_max_con(), _nodeId);


    _scheduler.enableAll();
}

void ICACHE_FLASH_ATTR painlessMesh::stop() {
    // Close all connections
    while (_connections.size() > 0) {
        auto connection = _connections.begin();
        (*connection)->close();
    }

    // Stop scanning task
    stationScan.task.setCallback(NULL);
    _scheduler.deleteTask(stationScan.task);

    // Note that this results in the droppedConnections not to be signalled
    // We might want to change this later
    newConnectionTask.setCallback(NULL);
    _scheduler.deleteTask(newConnectionTask);
    droppedConnectionTask.setCallback(NULL);
    _scheduler.deleteTask(droppedConnectionTask);

    // Shutdown wifi hardware
    WiFi.disconnect();
#ifdef ESP32
    esp_wifi_stop();
    esp_wifi_deinit();
#endif // ESP32
}

//***********************************************************************
// do nothing if user have other Scheduler, they have to run their scheduler in loop not this library
void ICACHE_FLASH_ATTR painlessMesh::update(void) {
    if (isExternalScheduler == false)
    {
        _scheduler.execute();
    }
    return;
}

//***********************************************************************
bool ICACHE_FLASH_ATTR painlessMesh::sendSingle(uint32_t &destId, String &msg) {
    debugMsg(COMMUNICATION, "sendSingle(): dest=%u msg=%s\n", destId, msg.c_str());
    return sendMessage(destId, _nodeId, SINGLE, msg);
}

//***********************************************************************
bool ICACHE_FLASH_ATTR painlessMesh::sendBroadcast(String &msg, bool includeSelf) {
    debugMsg(COMMUNICATION, "sendBroadcast(): msg=%s\n", msg.c_str());
    bool success = broadcastMessage(_nodeId, BROADCAST, msg);
    if (success && includeSelf && this->receivedCallback)
        this->receivedCallback(this->getNodeId(), msg);
    return success; 
}

bool ICACHE_FLASH_ATTR painlessMesh::startDelayMeas(uint32_t nodeId) {
    String timeStamp;
    debugMsg(S_TIME, "startDelayMeas(): NodeId %u\n", nodeId);

    auto conn = findConnection(nodeId);

    if (conn) {
        timeStamp = conn->time.buildTimeStamp(TIME_REQUEST, getNodeTime());
        //conn->timeDelayStatus = IN_PROGRESS;
    } else {
        return false;
    }
    debugMsg(S_TIME, "startDelayMeas(): Sent delay calc request -> %s\n", timeStamp.c_str());
    sendMessage(conn, nodeId, _nodeId, TIME_DELAY, timeStamp);
    //conn->timeSyncLastRequested = system_get_time();
    return true;
}
