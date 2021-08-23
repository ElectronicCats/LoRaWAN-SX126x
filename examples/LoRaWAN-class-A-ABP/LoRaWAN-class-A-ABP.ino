/**
   @file LoRaWAN_ABP.ino
   @author Andres Sabas @ ElectronicCats
   @brief LoRaWan node example with ABP registration
   @version 0.1
   @date 2021-01-12

*/
#include <LoRaWan-SX126x.h> //http://librarymanager/All#SX126x
#include <SPI.h>

// Define region REGION_US915, REGION_AS923, REGION_AU915,REGION_CN470, REGION_CN779,
// REGION_EU433, REGION_IN865, REGION_EU868, REGION_KR920, REGION_US915_HYBRID
#define REGION_US915

volatile bool flip = false;
volatile bool flag = false;

bool doOTAA = false;
#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE 60                      /**< Maximum number of events in the scheduler queue. */

#define LORAWAN_APP_DATA_BUFF_SIZE 64                     /**< buffer size of the data to be transmitted. */
#define LORAWAN_APP_INTERVAL 10000000                        /**< Defines for user timer, the application data transmission interval. 20s, value in [us]. */

#define LORAWAN_DATERATE DR_0                   /*LoRaMac datarates definition, from DR_0 to DR_5*/
#define LORAWAN_TX_POWER TX_POWER_5                 /*LoRaMac tx power definition, from TX_POWER_0 to TX_POWER_15*/
#define JOINREQ_NBTRIALS 3                      /**< Number of trials for the join request. */

DeviceClass_t gCurrentClass = CLASS_A;                /* class definition*/
lmh_confirm gCurrentConfirm = LMH_UNCONFIRMED_MSG;          /* confirm/unconfirm packet definition*/
uint8_t gAppPort = LORAWAN_APP_PORT;                /* data port*/

hw_config hwConfig;

// Microcontroller - SX126x pin configuration
int PIN_LORA_RESET = 10;  // LORA RESET
int PIN_LORA_DIO_1 = 9; // LORA DIO_1
int PIN_LORA_BUSY = 8;  // LORA SPI BUSY
int PIN_LORA_NSS = SS;   // LORA SPI CS
int PIN_LORA_SCLK = SCK;  // LORA SPI CLK
int PIN_LORA_MISO = MISO;  // LORA SPI MISO
int PIN_LORA_MOSI = MOSI;  // LORA SPI MOSI
int RADIO_TXEN = 4;   // LORA ANTENNA TX ENABLE
int RADIO_RXEN = 3;  // LORA ANTENNA RX ENABLE

/**@brief Structure containing LoRaWan parameters, needed for lmh_init()
*/
static lmh_param_t lora_param_init = {LORAWAN_ADR_OFF, LORAWAN_DATERATE, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

// Foward declaration
static void lorawan_has_joined_handler(void);
static void lorawan_rx_handler(lmh_app_data_t *app_data);
static void lorawan_confirm_class_handler(DeviceClass_t Class);
static void send_lora_frame(void);

/**@brief Structure containing LoRaWan callback functions, needed for lmh_init()
*/
static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
                                        lorawan_rx_handler, lorawan_has_joined_handler, lorawan_confirm_class_handler
                                       };

//ABP keys
uint32_t nodeDevAddr = 0x11111111;
uint8_t nodeNwsKey[16] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
uint8_t nodeAppsKey[16] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};


static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];        //< Lora user application data buffer.
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; //< Lora user application data structure.

TimerEvent_t appTimer;
static void timers_init(void);
static uint32_t count = 0;
static uint32_t count_fail = 0;

void setup()
{
  // Initialize Serial for debug output
  Serial.begin(9600);
  while (!Serial)
  Serial.println("=====================================");
  Serial.println("Welcome to Example with BastFrontier in LoRaWan!");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Define the HW configuration between MCU and SX126x
  hwConfig.CHIP_TYPE = SX1262_CHIP;     // Example uses an eByte E22 module with an SX1262
  hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
  hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;  // LORA SPI CS
  hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // LORA SPI CLK
  hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // LORA SPI MISO
  hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
  hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // LORA SPI BUSY
  hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // LORA SPI MOSI
  hwConfig.RADIO_TXEN = RADIO_TXEN;     // LORA ANTENNA TX ENABLE
  hwConfig.RADIO_RXEN = RADIO_RXEN;     // LORA ANTENNA RX ENABLE
  hwConfig.USE_DIO2_ANT_SWITCH = true;    // Example uses an CircuitRocks Alora RFM1262 which uses DIO2 pins as antenna control
  hwConfig.USE_DIO3_TCXO = true;        // Example uses an CircuitRocks Alora RFM1262 which uses DIO3 to control oscillator voltage
  hwConfig.USE_DIO3_ANT_SWITCH = false;  // Only Insight ISP4520 module uses DIO3 as antenna control
  hwConfig.USE_RXEN_ANT_PWR = true;

#if defined(REGION_AS923)
  Serial.println("Region: AS923");
#elif defined(REGION_AU915)
  Serial.println("Region: AU915");
#elif defined(REGION_CN470)
  Serial.println("Region: CN470");
#elif defined(REGION_CN779)
  Serial.println("Region: CN779");
#elif defined(REGION_EU433)
  Serial.println("Region: EU433");
#elif defined(REGION_IN865)
  Serial.println("Region: IN865");
#elif defined(REGION_EU868)
  Serial.println("Region: EU868");
#elif defined(REGION_KR920)
  Serial.println("Region: KR920");
#elif defined(REGION_US915)
  Serial.println("Region: US915");
#elif defined(REGION_US915_HYBRID)
  Serial.println("Region: US915_HYBRID");
#else
  Serial.println("Please define a region in the compiler options.");
#endif
  Serial.println("=====================================");

  // Initialize LoRa chip.
  if (!lora_hardware_init(hwConfig))
  {
    Serial.println("lora hardware init failed");
    while (true);
  }

  lmh_setNwkSKey(nodeNwsKey);
  lmh_setAppSKey(nodeAppsKey);
  lmh_setDevAddr(nodeDevAddr);

  // Initialize LoRaWan
  if (lmh_init(&lora_callbacks, lora_param_init, doOTAA))
  {
    Serial.println("lmh_init failed");
    while (true);
  }

  Serial.println("LoRaWAN ABP!!");
  Serial.println("=====================================");

  timers_init();
  
  // Start Join procedure
  lmh_join();
}

void loop()
{
  if (flag == true) {
    flip = !flip;
    digitalWrite(LED_BUILTIN, flip);
    Serial.println("send_lora_frame");
    if (lmh_join_status_get() != LMH_SET)
    {
      //Not joined, try again later
      return;
    }

    uint32_t i = 0;
    memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
    m_lora_app_data.port = gAppPort;
    m_lora_app_data.buffer[i++] = 'H';
    m_lora_app_data.buffer[i++] = 'e';
    m_lora_app_data.buffer[i++] = 'l';
    m_lora_app_data.buffer[i++] = 'l';
    m_lora_app_data.buffer[i++] = 'o';
    m_lora_app_data.buffer[i++] = '!';
    m_lora_app_data.buffsize = i;

    lmh_error_status error = lmh_send(&m_lora_app_data, LMH_UNCONFIRMED_MSG);
    if (error == LMH_SUCCESS)
    {
      count++;
      Serial.print("lmh_send ok count: ");
      Serial.println(count);
    }
    else
    {
      count_fail++;
      Serial.print("lmh_send fail count: ");
      Serial.println(count_fail);
    }
    
    flag = false;
  }

  // Handle Radio events
  Radio.IrqProcess();
}

/**@brief LoRa function for handling HasJoined event.
*/
void lorawan_has_joined_handler(void)
{
  Serial.println("ABP Mode Join!");

  lmh_error_status ret = lmh_class_request(gCurrentClass);
  Serial.print("lmh_error_status!");
  Serial.println(ret);
  
  if (ret == LMH_SUCCESS)
  {
    delay(1000);
    TimerSetValue(&appTimer, LORAWAN_APP_INTERVAL);
    TimerStart(&appTimer);
  }
}

/**@brief Function for handling LoRaWan received data from Gateway

   @param[in] app_data  Pointer to rx data
*/
void lorawan_rx_handler(lmh_app_data_t *app_data)
{
  
  Serial.println("lorawan_rx_handler");
  /*Serial.print("LoRa Packet received on port: ");
  Serial.println(app_data->port);
  Serial.print("LoRa Packet received on size:: ");
  Serial.println(app_data->buffsize);
  Serial.print("LoRa Packet received on rssi: ");
  Serial.println(app_data->rssi);
  Serial.print("LoRa Packet received on snr: ");
  Serial.println(app_data->snr);
  Serial.print("LoRa Packet received on data: ");
  */
  //Serial.println(app_data->buffer);
}

void lorawan_confirm_class_handler(DeviceClass_t Class)
{
  Serial.println("lorawan_confirm_class_handler");
  Serial.print("switch to class ");
  Serial.println(Class);
  
  // Informs the server that switch has occurred ASAP
  m_lora_app_data.buffsize = 0;
  m_lora_app_data.port = gAppPort;
  lmh_send(&m_lora_app_data, gCurrentConfirm);
}

void send_lora_frame(void)
{
  flag = true;
}

/**@brief Function for handling user timerout event.
*/
void tx_lora_periodic_handler(void)
{
  TimerSetValue(&appTimer, LORAWAN_APP_INTERVAL);
  TimerStart(&appTimer);
  send_lora_frame();
}

/**@brief Function for the Timer initialization.

   @details Initializes the timer module. This creates and starts application timers.
*/
void timers_init(void)
{
  appTimer.timerNum = 3;
  TimerInit(&appTimer, tx_lora_periodic_handler);
}
