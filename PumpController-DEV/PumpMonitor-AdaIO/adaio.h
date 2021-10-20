#define AIO_PPRESSURE_FEED   "pump.pump-pressure"
#define AIO_F1PRESSURE_FEED "pump.filter1-pressure"
#define AIO_F2PRESSURE_FEED "pump.filter2-pressure"
#define AIO_F3PRESSURE_FEED "pump.filter3-pressure"
#define AIO_PUMPTEMP_FEED "pump.pump-temperature"
#define USE_AIRLIFT     // required for Arduino Uno WiFi R2 board compatability
#include <AdafruitIO_WiFi.h>

AdafruitIO_WiFi aio(AIO_USERNAME, AIO_KEY, WIFI_SSID, WIFI_PASS, SPIWIFI_SS, SPIWIFI_ACK, SPIWIFI_RESET, NINA_GPIO0, &SPI);
AdafruitIO_Feed *ppressurefeed = aio.feed(AIO_PPRESSURE_FEED);
AdafruitIO_Feed *f1pressurefeed = aio.feed(AIO_F1PRESSURE_FEED);
AdafruitIO_Feed *f2pressurefeed = aio.feed(AIO_F1PRESSURE_FEED);
AdafruitIO_Feed *f3pressurefeed = aio.feed(AIO_F3PRESSURE_FEED);
AdafruitIO_Feed *ptempfeed = aio.feed(AIO_PUMPTEMP_FEED);
