#ifndef CONFIG_H
#define CONFIG_H

#define SLICE_DATA_INTERVAL_MS 1000

#define NUM_DCMT_SLICES 8
#define NUM_RLHT_SLICES 8
#define NUM_PHDO_SLICES 2

// microSD Card Reader connections
#define SD_CS 6
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

#define ESTOP 32

const char *ssid = "CAN'd-BREAD";
const char *password = "P@ssw0rd123#";

// CAN Ports
#define CAN_TX		19
#define CAN_RX		18
#define CAN_STDBY   17

#endif
