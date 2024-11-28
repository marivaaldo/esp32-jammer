#include "RF24.h"
#include <SPI.h>
#include <ezButton.h>
#include "esp_bt.h"
#include "esp_wifi.h"

#define HSPI_RADIO_SCK 14
#define HSPI_RADIO_MISO 12
#define HSPI_RADIO_MOSI 13
#define HSPI_RADIO_CS 15
#define HSPI_RADIO_CE 16

#define VSPI_RADIO_SCK 18
#define VSPI_RADIO_MISO 19
#define VSPI_RADIO_MOSI 23
#define VSPI_RADIO_CS 21
#define VSPI_RADIO_CE 22

#define CHANGE_STRATEGY_SWITCH_GPIO 33

#define SPI_SPEED 16000000 // (SPI Default Speed: 10000000)

#define TOTAL_STRATEGIES 2

void setupHSPIRadio();
void setupVSPIRadio();
void updateCurrentStrategy();
void executeCurrentStrategy();
void executeStrategy0();
void executeStrategy1();

SPIClass *vspiBus = nullptr;
SPIClass *hspiBus = nullptr;
RF24 hspiRadio(HSPI_RADIO_CE, HSPI_RADIO_CS, SPI_SPEED);
RF24 vspiRadio(VSPI_RADIO_CE, VSPI_RADIO_CS, SPI_SPEED);
ezButton changeStrategieswitch(CHANGE_STRATEGY_SWITCH_GPIO);

int currentStrategy = 0;
void (*strategies[])() = {executeStrategy0, executeStrategy1};
int hspiChannel = 45;
int vspiChannel = 45;

void setup()
{
  Serial.begin(115200);
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();
  changeStrategieswitch.setDebounceTime(50);
  setupHSPIRadio();
  setupVSPIRadio();
}

void loop()
{
  changeStrategieswitch.loop();
  updateCurrentStrategy();
  executeCurrentStrategy();
}

void setupHSPIRadio()
{
  hspiBus = new SPIClass(HSPI);
  hspiBus->begin();
  if (hspiRadio.begin(hspiBus))
  {
    Serial.println("HSPI Started!");
    hspiRadio.setAutoAck(false);
    hspiRadio.stopListening();
    hspiRadio.setRetries(0, 0);
    hspiRadio.setPALevel(RF24_PA_MAX, true);
    hspiRadio.setDataRate(RF24_2MBPS);
    hspiRadio.setCRCLength(RF24_CRC_DISABLED);
    hspiRadio.printPrettyDetails();
    hspiRadio.startConstCarrier(RF24_PA_MAX, hspiChannel);
  }
  else
  {
    Serial.println("ERROR while start HSPI");
  }
}

void setupVSPIRadio()
{
  vspiBus = new SPIClass(VSPI);
  vspiBus->begin();
  if (vspiRadio.begin(vspiBus))
  {
    Serial.println("VSPI Started!");
    vspiRadio.setAutoAck(false);
    vspiRadio.stopListening();
    vspiRadio.setRetries(0, 0);
    vspiRadio.setPALevel(RF24_PA_MAX, true);
    vspiRadio.setDataRate(RF24_2MBPS);
    vspiRadio.setCRCLength(RF24_CRC_DISABLED);
    vspiRadio.printPrettyDetails();
    vspiRadio.startConstCarrier(RF24_PA_MAX, vspiChannel);
  }
  else
  {
    Serial.println("ERROR while start VSPI");
  }
}

void updateCurrentStrategy()
{
  if (changeStrategieswitch.isPressed())
  {
    currentStrategy++;

    if (currentStrategy >= TOTAL_STRATEGIES)
      currentStrategy = 0;

    Serial.println("New Current Strategy: " + String(currentStrategy));

    delayMicroseconds(60);
  }
}

void executeCurrentStrategy()
{
  if (currentStrategy >= 0 && currentStrategy < TOTAL_STRATEGIES)
  {
    strategies[currentStrategy]();
  }
  else
  {
    delayMicroseconds(60);
  }
}

unsigned int strategy0_hspiDirection = 0; // For Strategy 0: 0 = increment, 1 = decrement
unsigned int strategy0_vspiDirection = 0; // For Strategy 0: 0 = increment, 1 = decrement

void executeStrategy0()
{
  if (strategy0_vspiDirection == 0)
  { // If strategy0_hspiDirection is 0, increment hspiChannel by 4 and vspiChannel by 1
    vspiChannel += 4;
  }
  else
  { // If strategy0_hspiDirection is not 0, decrement hspiChannel by 4 and vspiChannel by 1
    vspiChannel -= 4;
  }

  if (strategy0_hspiDirection == 0)
  { // If strategy0_hspiDirection is 0, increment hspiChannel by 4 and vspiChannel by 1
    hspiChannel += 2;
  }
  else
  { // If strategy0_hspiDirection is not 0, decrement hspiChannel by 4 and vspiChannel by 1
    hspiChannel -= 2;
  }

  // Check if vspiChannel is greater than 79 and strategy0_hspiDirection is 0
  if ((vspiChannel > 79) && (strategy0_vspiDirection == 0))
  {
    strategy0_vspiDirection = 1; // Set strategy0_hspiDirection to 1 to change direction
  }
  else if ((vspiChannel < 2) && (strategy0_vspiDirection == 1))
  {                              // Check if vspiChannel is less than 2 and strategy0_hspiDirection is 1
    strategy0_vspiDirection = 0; // Set strategy0_hspiDirection to 0 to change direction
  }

  // Check if hspiChannel is greater than 79 and strategy0_hspiDirection is 0
  if ((hspiChannel > 79) && (strategy0_hspiDirection == 0))
  {
    strategy0_hspiDirection = 1; // Set strategy0_hspiDirection to 1 to change direction
  }
  else if ((hspiChannel < 2) && (strategy0_hspiDirection == 1))
  {                              // Check if hspiChannel is less than 2 and strategy0_hspiDirection is 1
    strategy0_hspiDirection = 0; // Set strategy0_hspiDirection to 0 to change direction
  }

  hspiRadio.setChannel(hspiChannel);
  vspiRadio.setChannel(vspiChannel);
}

void executeStrategy1()
{
  vspiRadio.setChannel(random(80));
  hspiRadio.setChannel(random(80));
  delayMicroseconds(random(60));
  // for (int i = 0; i < 79; i++)
  // {
  //   hspiRadio.setChannel(i);
  //   vspiRadio.setChannel(random(80));
  // }
}
