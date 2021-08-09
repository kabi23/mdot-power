#include "select_program.h"

#if PROGRAM == DEPTH

#include "mDot.h"
#include "ChannelPlans.h"
#include "MTSText.h"
#include "device_addresses.h"



#define     SLEEP_S          30 * 60

mDot* dot = NULL;
lora::ChannelPlan* plan = NULL;

// sub band 1 - 8 will allow the radio to use the 8 channels in that sub band
static uint8_t frequency_sub_band = 1;

// deep sleep
static bool deep_sleep = false;

// retries 0 to disable acks
static uint8_t ack = 0;

// Parameters for ABP
std::vector<uint8_t> network_address_vector(network_address, network_address + 4);
std::vector<uint8_t> network_session_key_vector(network_session_key, network_session_key + 16);
std::vector<uint8_t> data_session_key_vector(data_session_key, data_session_key + 16);



static AnalogIn range_sensor(A0); // water level sensor
static AnalogIn vbat (PA_1); //battery voltage
static AnalogIn v_solar (A1); // solar voltage
static AnalogIn current_solar (A2); // solar current

const uint8_t DATA_RATE = 0;
const uint32_t TX_POWER = 14;





int main () {

  plan = new lora::ChannelPlan_EU868();
  assert(plan);

  mDot* dot = mDot::getInstance(plan);
  assert(dot);

  if (dot->getJoinMode() != mDot::MANUAL) {
    printf("\r\nChanging Join Mode to Manual\n\r");
    if (dot->setJoinMode(mDot::MANUAL) != mDot::MDOT_OK) {
      printf("\r\nChanging join mode failed!\n\r");
      return -1;
    }
  }
  // Set frequency subband
  if (dot->getFrequencySubBand() != frequency_sub_band) {
    if (dot->setFrequencySubBand(frequency_sub_band) != mDot::MDOT_OK) {
        printf("\r\nSetting frequency subband failed\n\r");
        return -1;
      }
  }

  // disable retries
  if (dot->setAck(ack) != mDot::MDOT_OK) {
    printf("\r\nSetting retries failed\n\r");
    return -1;
  }

  // Set ABP parameters
  if (dot->setNetworkAddress(network_address_vector) != mDot::MDOT_OK) {
    printf("\r\nSetting device address failed\n\r");
    return -1;
  }

  if (dot->setNetworkSessionKey(network_session_key_vector) != mDot::MDOT_OK) {
    printf("\r\nSetting network session key failed\n\r");
    return -1;
  }

  if (dot->setDataSessionKey(data_session_key_vector) != mDot::MDOT_OK) {
    printf("\r\nSetting application session key failed\n\r");
    return -1;
  }




  // Tx power
  dot->setTxPower(TX_POWER);

  // Data rate
  dot->setTxDataRate(DATA_RATE);

  while (1) {

    uint16_t height;
    uint16_t voltage;
    uint16_t solarv;
    uint16_t solarc;

    height = range_sensor.read_u16();
    voltage = vbat.read_u16();
    solarv = v_solar.read_u16();
    solarc = current_solar.read_u16();


    printf("\rLevel: %d\n, \rVBAT:%d\n , \rV_SOLAR:%d\n , \rC_SOLAR:%d\n", height, voltage , solarv, solarc);

    std::vector<uint8_t> tx_data;
    tx_data.push_back((height >> 8) & 0xFF);
    tx_data.push_back(height & 0xFF);
    tx_data.push_back((voltage >> 8) & 0xFF);
    tx_data.push_back(voltage & 0xFF);
    tx_data.push_back((solarv >> 8) & 0xFF);
    tx_data.push_back(solarv & 0xFF);
    tx_data.push_back((solarc >> 8) & 0xFF);
    tx_data.push_back(solarc & 0xFF);


    int32_t ret = dot->send(tx_data);
    if (ret != mDot::MDOT_OK) {
      printf("\r\nData transmission failed\n\r");
      // return -1;
    }
    // sleep
    dot->sleep(SLEEP_S, mDot::RTC_ALARM, deep_sleep);
  }
  
  return 0;
}


#endif
