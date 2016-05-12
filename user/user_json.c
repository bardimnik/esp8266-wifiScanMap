#include "user_json.h"
#include <mem.h>
#include "osapi.h"
#include "sync.h"

#define os_realloc pvPortRealloc
void *pvPortRealloc(void* ptr, size_t size);

#define MAX_SMALL_BUFFER_SIZE 255


#define os_sprintf  ets_sprintf

char * printmac(char *_out, uint8_t *buf) {
  os_sprintf(_out, "%02X:%02X:%02X:%02X:%02X:%02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
  return _out;
}

bool build_probe_json(struct probeinfo pi, bool second) {
  char buf [MAX_SMALL_BUFFER_SIZE];
  os_memset(buf, 0, MAX_SMALL_BUFFER_SIZE);
  char mac[17];
  printmac(mac, pi.bssid);
  if(second) {
    os_sprintf(buf, ",");
  }
  os_sprintf(buf, "%s{\"time_s\":%d,\"bssid\":\"%s\",\"essid\":\"%s\"}", buf, pi.time_s, mac, pi.ssid);
  return json_put_string(buf);
}

bool build_beacon_json(struct beaconinfo bi, bool second) {
  char buf [MAX_SMALL_BUFFER_SIZE];
  os_memset(buf, 0, MAX_SMALL_BUFFER_SIZE);
  char mac[17];
  printmac(mac, bi.bssid);
  if(second) {
    os_sprintf(buf, ",");
  }
  os_sprintf(buf, "%s{\"time_s\":%d,\"bssid\":\"%s\",\"essid\":\"%s\", \"channel\":%d, \"signal\":%d, \"encryption\":%d}", buf, bi.time_s, mac, bi.ssid, bi.channel, bi.rssi, bi.encryption);
  return json_put_string(buf);
}

bool build_client_json(struct clientinfo ci, bool second) {
  char buf[MAX_SMALL_BUFFER_SIZE];
  os_memset(buf, 0, MAX_SMALL_BUFFER_SIZE);
  char mac[17];
  printmac(mac, ci.bssid);
  if(second) {
    os_sprintf(buf, ",");
  }
  os_sprintf(buf, "%s{\"time_s\":%d,\"bssid\":\"%s\", \"signal\":%d}",buf,ci.time_s, mac, ci.rssi);
  return json_put_string(buf);
}

void build_beacons_json(fifo_t *_beacons) {
  json_put_char('[');  
  uint16_t i = 0;
  while(!fifo_isempty(_beacons)) {
    bool res;
    if(i == 0){
      res = build_beacon_json(fifo_pop(_beacons).beaconinfo,false);
    } else {
      res = build_beacon_json(fifo_pop(_beacons).beaconinfo, true);
      if(!res) {
        break;
      }
    }
    i++;
  }  
  json_put_char(']');
}

void build_probes_json(fifo_t *_probes) {
  json_put_char('[');  
  uint16_t i = 0;
  while(!fifo_isempty(_probes)) {
    bool res;
    if(i == 0){
      res = build_probe_json(fifo_pop(_probes).probeinfo, false);
    } else {
      res = build_probe_json(fifo_pop(_probes).probeinfo, true);
      if(!res) {
        break;
      }
    }
    i++;
  }  
  json_put_char(']');
}

void build_clients_json(fifo_t *_clients) {
  json_put_char('[');  
  uint16_t i = 0;
  while(!fifo_isempty(_clients)) {
    if(i == 0){
      build_client_json(fifo_pop(_clients).clientinfo,false);
    } else {
      build_client_json(fifo_pop(_clients).clientinfo, true);
    }
    i++;
  }  
  json_put_char(']');
}

void build_json(struct data *_data) {
  char tmp[MAX_SMALL_BUFFER_SIZE];
  os_sprintf(tmp, "{\"hostname\":\"esp8266_%d\",",spi_flash_get_id());
  json_put_string(tmp);
  json_put_string("\"probes\":");
  build_probes_json(&_data->probesinfos);
  if(json_put_string(",\"ap\":")) {
    build_beacons_json(&_data->beaconsinfos);
  }
  if( json_put_string(",\"stations\":") ) {
    build_clients_json(&_data->clientsinfos);
  }
  json_put_char('}');
}