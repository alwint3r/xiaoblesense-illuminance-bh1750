#include <Arduino.h>
#include <Wire.h>
#include <ArduinoBLE.h>

static constexpr auto lightsens_addr = 0x23;

static constexpr auto ONETIME_HRES = 0b00100000;
static constexpr auto ONETIME_HRES2 = 0b00100001;

static bool send_cmd(unsigned char cmd)
{
    Wire.beginTransmission(lightsens_addr);
    Wire.write(cmd);
    return Wire.endTransmission() == 0;
}

static float get_lux()
{
    Wire.beginTransmission(lightsens_addr);
    Wire.requestFrom(lightsens_addr, 2);
    uint16_t raw = Wire.read() << 8 | Wire.read();
    float result = (float)raw / 1.2f;
    Wire.endTransmission();

    return result;
}

static bool init_lux_sensor()
{
    Wire.beginTransmission(lightsens_addr);
    return Wire.endTransmission() == 0;
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    if (!init_lux_sensor())
    {
        Serial.println("Failed to init lux sensor");
        while (1)
            ;
    }

    if (!BLE.begin())
    {
        Serial.println("starting BLE failed!");
        while (1)
            ;
    }

    BLE.setLocalName("XIAO");
}

void loop()
{
    BLE.stopAdvertise();

    if (!send_cmd(ONETIME_HRES2))
    {
        Serial.println("Failed to send ONETIME_HRES measurement");
        delay(3000);
        return;
    }

    delay(200);

    float lux = get_lux();
    Serial.print(lux);
    Serial.println(" lux");

    BLEAdvertisingData adv_data;
    adv_data.setFlags(BLEFlagsBREDRNotSupported | BLEFlagsGeneralDiscoverable);

    unsigned long lux_int = lux * 100;
    unsigned char lux_bytes[4] = {0};

    memcpy(lux_bytes, &lux_int, sizeof(lux_int));

    adv_data.setAdvertisedServiceData(0x2AFB, lux_bytes, 3);

    BLE.setAdvertisingData(adv_data);
    BLE.advertise();

    delay(3000);
}