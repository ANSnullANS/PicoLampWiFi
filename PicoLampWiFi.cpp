#include <string.h>
#include <stdio.h>
#include <string.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "pico/unique_id.h"
#include "hardware/gpio.h"
#include "hardware/flash.h"

#include "pico/cyw43_arch.h"

#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/apps/httpd.h"

#include <tusb.h>
#include "Adafruit_NeoPixel.hpp"

#include "config.h"
#include "Rainbow.h"

#include "dhcpserver/dhcpserver.h"

#define DEBUG_printf printf

const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
uint8_t settings_data[FLASH_PAGE_SIZE];

typedef struct TCP_ASERVER_T_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
} TCP_SERVER_T;

// unique board id
char board_id_hex[16];

char ap_ssid[14];
char ap_password[9];
char aPasswordCalc[4];
char ap_prefix[] = "Lamp_";
// Status variables
Color oSelectedColor;
Color oDaylightColor = { 255, 255, 128 };
Color oFixedColor = { 0, 0, 0 };
int iBrightness = 250;
bool bIsRainbow = true;
bool bIsDaylight = false;
bool bSetColor = false;
bool bNoUpdate = false;

static mutex_t oLEDUpdateMutex;

// Timing-Variables
static uint32_t iNow = 0;
static uint32_t iLastStatusUpdate = 0;
static uint32_t iLastModeCheck = 0;
static uint32_t iServerTicks = 0;

Adafruit_NeoPixel oPixels = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Rainbow oRainbow;
LED_MODE eActiveMode;
LED_MODE ePreviousMode = MODE_UNKNOWN;
LED_MODE eDetectedMode;
bool bWiFiOverwrite = false;


pico_unique_board_id_t board_id;

dhcp_server_t dhcp_server;
ip4_addr_t oGateway, oMask;


#ifndef SSI_TAGS
#define SSI_TAGS
const char * ssi_tags[] = {
  "UID",
  "MODE",
  "COLOR",
  "BRIGHT"
};
#endif

static const char *cgi_handler_basic(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

u16_t ssi_stats_ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;

  switch(iIndex) {
    case 0: // UID
      printed = snprintf(pcInsert, iInsertLen, "%s", board_id_hex);
      break;
    case 1: // MODE
      printed = snprintf(pcInsert, iInsertLen, "%d", eActiveMode);
      break;
    case 2: // COLOR
      printed = snprintf(pcInsert, iInsertLen, "{ \"r\": %d, \"g\": %d, \"b\": %d }", oFixedColor.r, oFixedColor.g, oFixedColor.b);
      break;
    case 3: // BRIGHT
      printed = snprintf(pcInsert, iInsertLen, "%d", iBrightness);
      break;
    default: // unknown Tag
      printed = 0;
      break;

  }

  LWIP_ASSERT("sane length", printed <= 0xFFFF);
  return (u16_t)printed;
}


void ssi_ex_init(void) {
  int i;
  for (i = 0; i < LWIP_ARRAYSIZE(ssi_tags); i++) {
    LWIP_ASSERT("tag too long for LWIP_HTTPD_MAX_TAG_NAME_LEN", strlen(ssi_tags[i]) <= LWIP_HTTPD_MAX_TAG_NAME_LEN);
  }

  http_set_ssi_handler(ssi_stats_ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}


static const tCGI cgi_handlers[] = {
  {
    "/api",
    cgi_handler_basic
  }
};


void cgi_ex_init(void)
{
  http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
}


void print_buf(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}


void setBrightness(char *cLevel) {
  iBrightness = atoi(cLevel);

  #ifdef DEBUG
  printf("Level: ");
  printf("%d\n", iBrightness);
  #endif

  oPixels.setBrightness(iBrightness);
  oPixels.show();
}


void toggle_color() {
  oPixels.clear();
  for (int i=0; i<=NUM_PIXELS; i++) {
    oPixels.setPixelColor(i, oPixels.Color(oSelectedColor.r, oSelectedColor.g, oSelectedColor.b));
  }
  oPixels.show();
}


void save_settings() {
  // Check if live settings match data in flash.
  #ifdef DEBUG
  printf("Checking Flash Storage...");
  #endif
  bool bMismatchDetected = false;

  if (settings_data[ADR_FIXED_R] != flash_target_contents[ADR_FIXED_R]) {
    bMismatchDetected = true;
  }
  else if (settings_data[ADR_FIXED_G] != flash_target_contents[ADR_FIXED_G]) {
    bMismatchDetected = true;
  }
  else if (settings_data[ADR_FIXED_B] != flash_target_contents[ADR_FIXED_B]) {
    bMismatchDetected = true;
  }

  // Save Settings if not matching flash.
  if (bMismatchDetected) {
    #ifdef DEBUG
    printf("MISMATCH\nStoring settings...\n");
    #endif
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, settings_data, FLASH_PAGE_SIZE);
    #ifdef DEBUG
    printf("Reading Back Data:\n");
    print_buf(flash_target_contents, FLASH_PAGE_SIZE);
    #endif
    restore_interrupts (ints);
  }
#ifdef DEBUG
  else {
    printf("MATCH\n");
  }
#endif
}


void toggleFixedColor(char *cRed, char *cGreen, char *cBlue) {
  #ifdef DEBUG
  printf("Color: ");
  printf(cRed);
  printf(", ");
  printf(cGreen);
  printf(", ");
  printf(cBlue);
  printf("\n");
  #endif

  oFixedColor.r = atoi(cRed);
  oFixedColor.g = atoi(cGreen);
  oFixedColor.b = atoi(cBlue);

  oSelectedColor = oFixedColor;

  #ifdef DEBUG
  printf("Saving Fixed Color\n");
  #endif
  settings_data[ADR_FIXED_R] = oFixedColor.r;
  settings_data[ADR_FIXED_G] = oFixedColor.g;
  settings_data[ADR_FIXED_B] = oFixedColor.b;

  save_settings();

  toggle_color();
}


static const char * cgi_handler_basic(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  LWIP_ASSERT("check index", iIndex < LWIP_ARRAYSIZE(cgi_handlers));

  #ifdef DEBUG
  printf("\nParams: ");
  printf("%d\n", iNumParams);
  #endif

 if(!strcmp(pcParam[0], "act")) {
  if(!strcmp(pcValue[0], "rain")) {
    eActiveMode = MODE_RAINBOW;
  }
  else if(!strcmp(pcValue[0], "bri")) {
    setBrightness(pcValue[1]);
  }
  else if(!strcmp(pcValue[0], "fix")) {
    eActiveMode = MODE_FIXED;
    toggleFixedColor(pcValue[1], pcValue[2], pcValue[3]);
  }
  else if(!strcmp(pcValue[0], "day")) {
    eActiveMode = MODE_DAYLIGHT;
    oSelectedColor = oDaylightColor;
    toggle_color();
  }
  else {
    return "/404.html";
  }

  return "/stats.json";
 }
 else {
  return "/404.html";
 }

  
}


void showStatus(Color oColor) {
  // Use Pixels for Status
  bNoUpdate = true;
  oPixels.clear();
  oPixels.setPixelColor(0, oPixels.Color(oColor.r,oColor.g,oColor.b));
  oPixels.show();
  sleep_ms(1000);
  bNoUpdate = false;
}

bool connectWifi() { 
  printf("\nSSID: ");
  printf(ap_ssid);
  printf("\nPassword: ");
  printf(ap_password);
  printf("\n\n");

  cyw43_arch_enable_ap_mode(ap_ssid, ap_password, CYW43_AUTH_WPA2_AES_PSK);

  return true;
}


void sendUpdate() {
  if(mutex_enter_timeout_ms(&oLEDUpdateMutex, 100))
  {
    for(int i=0; i < NUM_PIXELS; i++) {
      oPixels.setPixelColor(i, oPixels.Color(oRainbow.CHA_Values[i].r, oRainbow.CHA_Values[i].g, oRainbow.CHA_Values[i].b));
    }
    oPixels.show();

    mutex_exit(&oLEDUpdateMutex);
  }
}


void checkMode() {
  bIsDaylight = gpio_get(DAYLIGHT_PIN);
  bIsRainbow = gpio_get(RAINBOW_PIN);
  if (!bIsDaylight && !bIsRainbow) {
    eDetectedMode = MODE_FIXED;
  }
  else if(bIsDaylight) {
    eDetectedMode = MODE_DAYLIGHT;
  }
  else if(bIsRainbow) {
    eDetectedMode = MODE_RAINBOW;
  }

  if (eDetectedMode != ePreviousMode) {
    eActiveMode = eDetectedMode;
    ePreviousMode = eDetectedMode;

    switch(eActiveMode) {
      case MODE_DAYLIGHT:
        printf("Toggle Daylight\n");
        oSelectedColor = oDaylightColor;
        toggle_color();
        break;
      case MODE_FIXED:
        printf("Toggle Fixed\n");
        oSelectedColor = oFixedColor;
        toggle_color();
        break;
      case MODE_RAINBOW:
        printf("Toggle Rainbow\n");
        break;
    }
  }
}


void core1_entry() {
  // Configure LEDS
  printf("Configuring LEDs...");
  oPixels.begin();
  oPixels.clear();
  oPixels.setBrightness(iBrightness);
  oPixels.show();
  printf("OK\n");
  sleep_ms(300);

  checkMode();

  for(;;) {
    if(bNoUpdate) {
      continue;
    }

    iNow = to_ms_since_boot(get_absolute_time());

    if(eActiveMode == MODE_RAINBOW) {
      oRainbow.updateMultiplier(iNow);

      oRainbow.calculateValues();
      sendUpdate();

      // Update LED-Positions
      oRainbow.iCurrent = oRainbow.safeIncrement(oRainbow.iCurrent, NUM_PIXELS - 1);

      // Update Rainbow-Positions
      if(oRainbow.iMulti >= RAINBOW_UPDATE_INTERVAL) {
        oRainbow.iRainbow = oRainbow.safeIncrement(oRainbow.iRainbow, RAINBOWMAX - 1);
        oRainbow.iRainbowNext = oRainbow.safeIncrement(oRainbow.iRainbowNext, RAINBOWMAX - 1);

        oRainbow.lLastUpdate = iNow;
      }

      sleep_ms(10);
    }
    else {
      sleep_ms(300);
    }
  }
}


void run_server() {
  dhcp_server_init(&dhcp_server, &oGateway, &oMask);

  sleep_ms(10);

  httpd_init();
  ssi_ex_init();
  cgi_ex_init(); 
  
  printf("Webserver Ready");
  for(;;) {
      iServerTicks = to_ms_since_boot(get_absolute_time());
      if (iServerTicks >= iLastStatusUpdate + STATUS_UPDATE_INTERVAL) {
        printf(".");
        iLastStatusUpdate = iServerTicks;
      }

      // Check Switch Position
      if (iServerTicks >= iLastModeCheck + MODE_CHECK_INTERVAL) {
        iLastModeCheck = iServerTicks;
        checkMode();
      }
      
      sleep_ms(10);
  }
};


void load_settings() {
  oFixedColor.r = flash_target_contents[ADR_FIXED_R];
  oFixedColor.g = flash_target_contents[ADR_FIXED_G];
  oFixedColor.b = flash_target_contents[ADR_FIXED_B];

  #ifdef DEBUG
  printf("\nFixed R: ");
  printf("%2d", oFixedColor.r);
  printf("\nFixed G: ");
  printf("%2d", oFixedColor.g);
  printf("\nFixed B: ");
  printf("%2d", oFixedColor.b);
  printf("\n");
  #endif
}


int main() {
    bi_decl(bi_program_description("PicoLamp"));
    bi_decl(bi_1pin_with_name(LED_PIN, "Data-Pin for LED-Stripe."));

    IP4_ADDR(&oGateway, 192, 168, 4, 1);
    IP4_ADDR(&oMask, 255, 255, 255, 0);

    mutex_init(&oLEDUpdateMutex);

    stdio_init_all();

    printf("\n\nPicoLampWiFi v");
    printf(VERSION_NO);
    printf(" starting...\n\n");

    // Get Board UID
    pico_get_unique_board_id(&board_id);
    printf("Unique identifier: ");
    sprintf(board_id_hex, "%02x%02x%02x%02x%02x%02x%02x%02x", board_id.id[0], board_id.id[1], board_id.id[2], board_id.id[3], board_id.id[4], board_id.id[5], board_id.id[6], board_id.id[7]);
    printf(board_id_hex);
    printf("\n\n");

    // Calculate Password
    aPasswordCalc[0] = board_id.id[0] + board_id.id[7];
    aPasswordCalc[1] = board_id.id[1] + board_id.id[6];
    aPasswordCalc[2] = board_id.id[2] + board_id.id[5];
    aPasswordCalc[3] = board_id.id[3] + board_id.id[4];

    // Generate SSID & Password-String
    sprintf(ap_ssid, "%s%02x%02x%02x%02x", ap_prefix, board_id.id[4], board_id.id[5], board_id.id[6], board_id.id[7]);
    sprintf(ap_password, "%02x%02x%02x%02x", aPasswordCalc[0], aPasswordCalc[0], aPasswordCalc[2], aPasswordCalc[3]);// snprintf("%02x%02x%02x%02x", board_id.id[7], board_id.id[6], board_id.id[5], board_id.id[4])

#ifdef DEBUG
    // Show WiFi Information on Serial
    printf("SSID: ");
    printf(ap_ssid);
    printf("\nPassword: ");
    printf(ap_password);
    printf("\n\n");
#endif

    // Configure Pins
    gpio_init(RAINBOW_PIN);
    gpio_set_dir(RAINBOW_PIN, GPIO_IN);
    gpio_init(DAYLIGHT_PIN);
    gpio_set_dir(DAYLIGHT_PIN, GPIO_IN);

    // Restore Settings
#ifdef DEBUG
    printf("Loading Settings...\n");
    print_buf(flash_target_contents, FLASH_PAGE_SIZE);
#endif
    load_settings();

    multicore_launch_core1(core1_entry);

    if (cyw43_arch_init()) {
      printf("failed to initialise\n");
      return 1;
    }

    showStatus({255,255,0});
     
    printf("Starting WiFi-AP...\n");
    if (!connectWifi()) {
      return 1;
    }

    run_server();

    cyw43_arch_deinit();
    return 0;
};