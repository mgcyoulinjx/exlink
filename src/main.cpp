#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Wire.h>
#include "CST816T.h"
#include <WiFi.h>
#include "ui.h"
#include "INA.h"
#include "stdio.h"
#include "driver/ledc.h"
#include "driver/pcnt.h"
#include "soc/pcnt_struct.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_task_wdt.h>

#define I2C_SDA 18 // 瑙︽懜SDA寮曡剼
#define I2C_SCL 21 // 瑙︽懜SCL寮曡剼
#define RST_N_PIN 16
#define INT_N_PIN 18
#define RIGHT_PIN 38
#define LEFT_PIN 39
#define PUSH_PIN 40
#define PCNT_UNIT PCNT_UNIT_0                                         // 浣跨敤PCNT鍗曞厓0
#define SHORT_PRESS_THRESHOLD 10                                      // 定义短按时间阈值
#define DOUBLE_CLICK_THRESHOLD 250                                    // 定义双击时间阈值
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"           // 鏈嶅姟UUID
#define CHARACTERISTIC_UUID_RX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // 鎺ユ敹鐗瑰緛UUID
#define CHARACTERISTIC_UUID_TX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // 鍙戦€佺壒寰乁UID

int adcValues[SAMPLE_COUNT];                                   // 瀛樺偍ADC閲囨牱鏁板€?
char maxValueStr[20], minValueStr[20], peakToPeakValueStr[20]; // 瀛樻渶澶ф渶灏忓嘲宄板€?

volatile int pulseCount = 0;
float frequency = 0.0;
char freqencyStr[20];
int16_t count = 0;

unsigned long previousMillis = 0; // 璁板綍涓婃璇诲彇璁℃暟鐨勬椂闂?

const long interval = 125; // 璁剧疆鑴夊啿璇诲彇闂撮殧涓?25姣

INA_Class INA;
int8_t request_index = 0;
char voltageStr[20], currentStr[20], powerStr[20], mAHStr[20]; // 瀛樺偍ina226娴嬮噺鏁版嵁
uint64_t vol, cur, wat;
double v, a, w;
volatile uint8_t deviceNumber = UINT8_MAX;
volatile uint64_t sumBusMillVolts = 0;
volatile int64_t sumBusMicroAmps = 0;
volatile uint8_t readings = 0;
volatile int64_t mAH = 0;

int ina266_flag = 0;
int mcp4107_flag = 0;
int pwm_flag = 0;
int i2cscan_flag = 0;
int uart_helper_flag = 0;
int DSO_flag = 0;
int BluetoothSerial_flag = 0;
int FREcount_flag = 0;

int pwm_Freq = 0;
int pwm_Duty = 0;

int new_pwm_Freq = 0;
int new_pwm_Duty = 0;

int mcp4107_value;
long currentBaudRate = 115200; // 瀹氫箟鍒濆娉㈢壒鐜?
lv_indev_t *indev_keypad;
byte error, address;
int nDevices;
// 鍦ㄨ繖閲岃缃睆骞曞昂瀵?
static const uint32_t screenWidth = TFT_WIDTH;
static const uint32_t screenHeight = TFT_HEIGHT;
// lvgl鏄剧ず瀛樺偍鏁扮粍
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 2];
CST816T cst816t(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN);
hw_timer_t *tim1 = NULL;
TFT_eSPI tft = TFT_eSPI();

BLEServer *pServer = NULL;            // BLEServer鎸囬拡pServer
BLECharacteristic *pTxCharacteristic; // BLECharacteristic鎸囬拡 pTxCharacteristic
BLECharacteristic *pRxCharacteristic; // BLECharacteristic鎸囬拡 pRxCharacteristic
bool deviceConnected = false;         // 鏈杩炴帴鐘舵€?
bool oldDeviceConnected = false;      // 涓婃杩炴帴鐘舵€?
uint8_t txValue = 0;

class MyServerCallbacks : public BLEServerCallbacks // BLE杩炴帴鍥炶皟鍑芥暟
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks // BLE鎺ユ敹鍥炶皟鍑芥暟
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.println(rxValue[i]);
      }
    }
  }
};

void mcp4017_init() // 鏁板瓧鐢典綅鍣ㄥ垵濮嬪寲
{
  Wire.beginTransmission(0x2F);
  Wire.write(0x56);
  Wire.endTransmission();
}

void mcp4017_task() // 鏁板瓧鐢典綅鍣ㄤ换鍔?
{
  if (mcp4107_flag == 1)
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value = 0x10;
    Wire.write(mcp4107_value);
    Wire.endTransmission();
  }
  if (mcp4107_flag == 2)
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value = 0x2C;
    Wire.write(mcp4107_value);
    Wire.endTransmission();
  }
  if (mcp4107_flag == 3)
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value = 0x56;
    Wire.write(mcp4107_value);
    Wire.endTransmission();
  }

  if (mcp4107_flag == 4)
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value--;
    if (mcp4107_value <= 0x01)
    {
      mcp4107_value = 0x01;
    }
    Wire.write(mcp4107_value);
    Wire.endTransmission();
    mcp4107_flag = 0;
  }

  if (mcp4107_flag == 5)
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value++;
    if (mcp4107_value >= 0x7E)
    {
      mcp4107_value = 0x7E;
    }
    Wire.write(mcp4107_value);
    Wire.endTransmission();
    mcp4107_flag = 0;
  }
}

// 闈為樆濉炶渹楦ｅ櫒鎺у埗
static uint32_t buzzer_start_time = 0;
static bool buzzer_active = false;
#define BUZZER_DURATION 50  // 铚傞福鍣ㄦ寔缁椂闂?ms)

void buzzer_beep() // 瑙﹀彂铚傞福鍣紙闈為樆濉烇級
{
  if (!buzzer_active)
  {
    ledcWrite(1, 3);
    buzzer_active = true;
    buzzer_start_time = millis();
  }
}

void buzzer_update() // 鏇存柊铚傞福鍣ㄧ姸鎬侊紙鍦╨oop涓皟鐢級
{
  if (buzzer_active && (millis() - buzzer_start_time >= BUZZER_DURATION))
  {
    ledcWrite(1, 0);
    buzzer_active = false;
  }
}

void pwm_task() // pwm杈撳嚭浠诲姟
{

  if (pwm_flag == 1)
  {

    const char *freq_str = lv_textarea_get_text(fre);
    const char *duty_str = lv_textarea_get_text(duty);
    pwm_Freq = atoi(freq_str);
    pwm_Duty = constrain(atoi(duty_str), 0, 100); // 闄愬埗鍗犵┖姣斿湪 0-100 涔嬮棿
    pwm_Duty = map(pwm_Duty, 0, 100, 0, 255);

    if (pwm_Freq != new_pwm_Freq | pwm_Duty != new_pwm_Duty)
    {
      ledcAttachPin(5, 2);       // 灏嗗紩鑴氳繛鎺ュ埌閫氶亾2
      ledcSetup(2, pwm_Freq, 8); // 璁剧疆閫氶亾2涓?浣嶅垎杈ㄧ巼
      ledcWrite(2, pwm_Duty);    // 閫氶亾2杈撳嚭
      new_pwm_Freq = pwm_Freq;
      new_pwm_Duty = pwm_Duty;
    }
  }

  if (pwm_flag == 0)
  {
    new_pwm_Freq = 0;
    new_pwm_Duty = 0;
    ledcWrite(2, 0);
  }
}

void i2cscan_task() // I2C鎵弿浠诲姟
{
  if (i2cscan_flag == 1)
  {

    Wire.begin(6, 7);
    lv_textarea_add_text(i2c_extarea, "Scanning...\n");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
      Wire1.begin(6, 7);
      Wire1.beginTransmission(address); // 浠庢寚瀹氱殑鍦板潃寮€濮嬪悜I2C浠庤澶囪繘琛屼紶杈?
      error = Wire1.endTransmission();  // 鍋滄涓庝粠鏈虹殑鏁版嵁浼犺緭
      if (error == 0)
      {
        char i2c_address[20];
        lv_textarea_add_text(i2c_extarea, "I2C device address 0x");
        if (address < 16)
          lv_textarea_add_text(i2c_extarea, "0");
        sprintf(i2c_address, "%X\n", address);
        lv_textarea_add_text(i2c_extarea, i2c_address);

        nDevices++;
      }
      else if (error == 4)
      {
        char i2c_address[20];
        lv_textarea_add_text(i2c_extarea, "Unknown error at address 0x");
        if (address < 16)
          lv_textarea_add_text(i2c_extarea, "0");
        sprintf(i2c_address, "%X\n", address);
        lv_textarea_add_text(i2c_extarea, i2c_address);
      }
    }
    if (nDevices == 0)
      lv_textarea_add_text(i2c_extarea, "No I2C devices found");
    else
      lv_textarea_add_text(i2c_extarea, "done\n");

    i2cscan_flag = 0;
  }
}

void uart_helper_task() // 涓插彛鍔╂墜浠诲姟
{

  if (uart_helper_flag == 1)
  {

    char buf[10];
    lv_dropdown_get_selected_str(uart_list, buf, sizeof(buf));
    long newBaudRate = 0;

    if (strcmp(buf, "1200") == 0)
    {
      newBaudRate = 1200;
    }
    else if (strcmp(buf, "2400") == 0)
    {
      newBaudRate = 2400;
    }
    else if (strcmp(buf, "4800") == 0)
    {
      newBaudRate = 4800;
    }
    else if (strcmp(buf, "9600") == 0)
    {
      newBaudRate = 9600;
    }
    else if (strcmp(buf, "19200") == 0)
    {
      newBaudRate = 19200;
    }
    else if (strcmp(buf, "43000") == 0)
    {
      newBaudRate = 43000;
    }
    else if (strcmp(buf, "76800") == 0)
    {
      newBaudRate = 76800;
    }
    else if (strcmp(buf, "115200") == 0)
    {
      newBaudRate = 115200;
    }
    else if (strcmp(buf, "128000") == 0)
    {
      newBaudRate = 128000;
    }
    else if (strcmp(buf, "230400") == 0)
    {
      newBaudRate = 230400;
    }
    else if (strcmp(buf, "256000") == 0)
    {
      newBaudRate = 256000;
    }
    else if (strcmp(buf, "460800") == 0)
    {
      newBaudRate = 460800;
    }
    else if (strcmp(buf, "921600") == 0)
    {
      newBaudRate = 921600;
    }
    if (newBaudRate != currentBaudRate)
    {
      Serial.end();                  // 缁撴潫褰撳墠涓插彛
      Serial.begin(newBaudRate);     // 浠ユ柊娉㈢壒鐜囧紑濮嬩覆鍙?
      currentBaudRate = newBaudRate; // 鏇存柊褰撳墠娉㈢壒鐜?
    }
    if (Serial.available() > 0)
    {
      String input = Serial.readStringUntil('\n');
      lv_textarea_add_text(uart_extarea, input.c_str());
    }
  }
}

void DSO_task() // 绠€鏄撶ず娉㈠櫒浠诲姟
{

  if (DSO_flag == 1)
  {
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
      adcValues[i] = analogRead(4); // Read ADC values
      lv_chart_set_next_value(DSO_chart, DSO_ser, adcValues[i]);
    }

    float maxValue;
    float minValue = 4095;
    float peakToPeakValue;

    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
      if (adcValues[i] > maxValue)
      {
        maxValue = adcValues[i];
      }
      if (adcValues[i] < minValue)
      {
        minValue = adcValues[i];
      }
    }
    maxValue = maxValue / 4095 * 3.3;
    minValue = minValue / 4095 * 3.3;

    peakToPeakValue = maxValue - minValue; // 璁＄畻宄板嘲鍊?

    sprintf(maxValueStr, "%0.2f", maxValue);
    sprintf(minValueStr, "%0.2f", minValue);
    sprintf(peakToPeakValueStr, "%0.2f", peakToPeakValue);
  }
}

void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) // 按键扫描函数（PUSH_PIN 支持双击返回）
{
  (void)indev_drv;
  static uint32_t last_key = 0;
  static uint32_t push_start_time = 0;
  static uint32_t last_push_click_time = 0;
  static bool push_pressed = false;
  static uint8_t last_key_state = 0;
  uint8_t key_state = 0;
  uint32_t act_key = last_key;

  if (digitalRead(RIGHT_PIN) == LOW)
  {
    key_state = 1;
  }
  else if (digitalRead(LEFT_PIN) == LOW)
  {
    key_state = 2;
  }
  else if (digitalRead(PUSH_PIN) == LOW)
  {
    if (!push_pressed)
    {
      push_pressed = true;
      push_start_time = millis();
    }
    key_state = 3;
  }
  else
  {
    key_state = 0;
    if (push_pressed)
    {
      uint32_t now = millis();
      uint32_t press_duration = now - push_start_time;
      if (press_duration >= SHORT_PRESS_THRESHOLD)
      {
        if (last_push_click_time != 0 && (now - last_push_click_time) <= DOUBLE_CLICK_THRESHOLD)
        {
          buzzer_beep();
          lv_event_send(lv_scr_act(), LV_EVENT_LONG_PRESSED, NULL);
          last_push_click_time = 0;
        }
        else
        {
          last_push_click_time = now;
        }
      }
    }
    push_pressed = false;
  }

  if (key_state != 0)
  {
    data->state = LV_INDEV_STATE_PR;

    switch (key_state)
    {
    case 1:
      act_key = LV_KEY_NEXT;
      last_key = act_key;
      break;
    case 2:
      act_key = LV_KEY_PREV;
      last_key = act_key;
      break;
    case 3:
      if (push_pressed)
      {
        act_key = LV_KEY_ENTER;
        last_key = act_key;
      }
      break;
    default:
      break;
    }

    if (key_state != last_key_state)
    {
      buzzer_beep();
    }
    last_key_state = key_state;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
    last_key_state = 0;
  }

  data->key = last_key;
}

void ina266_task() // 鍔熺巼鐩戞祴浠诲姟
{

  if (ina266_flag == 1)
  {
    vol = INA.getBusMilliVolts(deviceNumber);
    cur = INA.getBusMicroAmps(deviceNumber) / 5000;
    wat = INA.getBusMicroWatts(deviceNumber);

    if (cur > 20 * 1000)
    {
      cur = 0;
    }

    v = vol / 1000.0;
    a = cur / 1000.0;
    w = v * a;

    if (v > 10)
    {
      sprintf(voltageStr, "%0.2f", v);
    }
    else
    {
      sprintf(voltageStr, "%0.3f", v);
    }

    if (a > 10)
    {
      sprintf(currentStr, "%0.2f", a);
    }
    else
    {
      sprintf(currentStr, "%0.3f", a);
    }

    if (w > 10)
    {
      sprintf(powerStr, "%0.2f", w);
    }
    else
    {
      sprintf(powerStr, "%0.3f", w);
    }

    mAH += cur;
    sprintf(mAHStr, "%0.3f", mAH / (60 * 60) / 1000.0);
  }
}

void BluetoothSerial_task() // 鏃犵嚎涓插彛浠诲姟锛堝熀浜嶣LE瀹炵幇锛?
{
  if (BluetoothSerial_flag == 1)
  {
    BLEDevice::init("Exlink"); // 鍒涘缓涓€涓狟LE璁惧

    pServer = BLEDevice::createServer();            // 鍒涘缓涓€涓狟LE鏈嶅姟
    pServer->setCallbacks(new MyServerCallbacks()); // 璁剧疆鍥炶皟

    BLEService *pService = pServer->createService(SERVICE_UUID); // 鍒涘缓钃濈墮鏈嶅姟鍣?

    // 鍒涘缓鍙戦€佺壒寰侊紝娣诲姞鎻忚堪绗︼紝璁剧疆閫氱煡鏉冮檺
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    // 鍒涘缓鎺ユ敹鐗瑰緛锛岃缃洖璋冨嚱鏁帮紝璁剧疆鍙啓鏉冮檺
    pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();                  // 鍚姩鏈嶅姟
    pServer->getAdvertising()->start(); // 寮€濮嬪箍鎾?
    BluetoothSerial_flag = 2;
  }
  if (BluetoothSerial_flag == 2)
  {
    char buf[10];
    lv_dropdown_get_selected_str(wireless_uart_list, buf, sizeof(buf));
    long newBaudRate = 0;

    if (strcmp(buf, "1200") == 0)
    {
      newBaudRate = 1200;
    }
    else if (strcmp(buf, "2400") == 0)
    {
      newBaudRate = 2400;
    }
    else if (strcmp(buf, "4800") == 0)
    {
      newBaudRate = 4800;
    }
    else if (strcmp(buf, "9600") == 0)
    {
      newBaudRate = 9600;
    }
    else if (strcmp(buf, "19200") == 0)
    {
      newBaudRate = 19200;
    }
    else if (strcmp(buf, "43000") == 0)
    {
      newBaudRate = 43000;
    }
    else if (strcmp(buf, "76800") == 0)
    {
      newBaudRate = 76800;
    }
    else if (strcmp(buf, "115200") == 0)
    {
      newBaudRate = 115200;
    }
    else if (strcmp(buf, "128000") == 0)
    {
      newBaudRate = 128000;
    }
    else if (strcmp(buf, "230400") == 0)
    {
      newBaudRate = 230400;
    }
    else if (strcmp(buf, "256000") == 0)
    {
      newBaudRate = 256000;
    }
    else if (strcmp(buf, "460800") == 0)
    {
      newBaudRate = 460800;
    }
    else if (strcmp(buf, "921600") == 0)
    {
      newBaudRate = 921600;
    }
    if (newBaudRate != currentBaudRate)
    {
      Serial.end();                  // 缁撴潫褰撳墠涓插彛
      Serial.begin(newBaudRate);     // 浠ユ柊娉㈢壒鐜囧紑濮嬩覆鍙?
      currentBaudRate = newBaudRate; // 鏇存柊褰撳墠娉㈢壒鐜?
    }

    if (deviceConnected) // 杩炴帴鏃舵墽琛屼覆鍙ｈ浆鍙?
    {
      if (Serial.available() > 0)
      {
        String strValue = Serial.readStringUntil('\n');
        pTxCharacteristic->setValue((uint8_t *)strValue.c_str(), strValue.length());
        pTxCharacteristic->notify();
        lv_textarea_add_text(wireless_uart_extarea, strValue.c_str());
        delay(100);
      }
    }

    if (!deviceConnected && oldDeviceConnected) // 鏈繛鎺ユ椂鎵ц钃濈墮骞挎挱
    {
      delay(500);
      pServer->startAdvertising();
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) // 鏇存柊钃濈墮鐘舵€?
    {
      oldDeviceConnected = deviceConnected;
    }
  }
}

void FREcount_task() // 鏁板瓧棰戠巼璁′换鍔?
{
  if (FREcount_flag == 1)
  {
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = 5,                // 杈撳叆寮曡剼
        .ctrl_gpio_num = PCNT_PIN_NOT_USED, // 涓嶄娇鐢ㄦ帶鍒跺紩鑴?
        .lctrl_mode = PCNT_MODE_KEEP,       // 鑴夊啿璁℃暟妯″紡
        .hctrl_mode = PCNT_MODE_KEEP,       // 绂佺敤楂樼數骞宠鏁?
        .pos_mode = PCNT_COUNT_INC,         // 姝ｈ剦鍐茶鏁?
        .neg_mode = PCNT_COUNT_DIS,         // 绂佺敤璐熻剦鍐茶鏁?
        .unit = PCNT_UNIT,                  // 浣跨敤鐨凱CNT鍗曞厓
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config); // 閰嶇疆PCNT鍗曞厓
    pcnt_counter_clear(PCNT_UNIT);  // 鍚姩璁℃暟
    FREcount_flag = 2;
  }
  if (FREcount_flag == 2)
  {
    unsigned long currentMillis = millis();
    pcnt_get_counter_value(PCNT_UNIT, &count);
    unsigned long currentinterval = currentMillis - previousMillis;
    if (currentinterval >= interval) // 濡傛灉杈惧埌璇诲彇闂撮殧
    {

      //  璁＄畻棰戠巼锛堝亣璁炬瘡娆¤鍙栫殑鏃堕棿涓?绉掞級
      //  unsigned long frequency = count/currentinterval*1000; // 棰戠巼 = 璁℃暟鍊硷紙璧吂锛?
      sprintf(freqencyStr, "%d", count * 8); // 娓呴浂璁℃暟
      pcnt_counter_clear(PCNT_UNIT);         // 鏇存柊鏁版嵁
      previousMillis = currentMillis;        // 鏇存柊涓婃璇诲彇璁℃暟鐨勬椂闂?
    }
  }
}

// 灞忓箷鎵撶偣鍑芥暟
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// 瑙︽懜灞忓洖璋冨嚱鏁?
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  TouchInfos tp;
  tp = cst816t.GetTouchInfo();
  bool touched = tp.touching;
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
    // Serial.println("NO TOUCH");
  }
  if (touched)
  {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = tp.y;
    data->point.y = TFT_WIDTH - tp.x;
    // Serial.println("TOUCHING");
  }
}

// 瀹氭椂鍣ㄤ腑鏂湇鍔″嚱鏁?
void tim1Interrupt()
{
  lv_tick_inc(1);
}

// lvgl浠诲姟澶勭悊鍑芥暟
void lvgl_handler(void *pvParameters)
{
  while (1)
  {
    lv_timer_handler(); /* let the GUI do its work */
    esp_task_wdt_reset(); /* 鍠傜湅闂ㄧ嫍锛岄槻姝㈤噸鍚?*/
    delay(5);
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  pinMode(3, OUTPUT);
  ledcAttachPin(3, 1);
  ledcSetup(1, 1000, 8);
  ledcWrite(1, 0);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(PUSH_PIN, INPUT_PULLUP);
  // analogReadResolution(12);            
  // analogSetPinAttenuation(2, ADC_11db); 
  // analogSetPinAttenuation(4, ADC_11db); 
  tft.init();
  tft.setRotation(3); // 璁剧疆鏄剧ず鏂瑰悜
  // cst816t.begin();    // 鍒濆鍖栬Е鎽稿睆

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 2);
  static lv_disp_drv_t disp_drv; // 鍒濆鍖栨樉绀哄櫒

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = tft.width();
  disp_drv.ver_res = tft.height();
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.sw_rotate = 0;             // 灞忓箷闀滃儚
  disp_drv.rotated = LV_DISP_ROT_NONE; // 灞忓箷鏃嬭浆
  lv_disp_drv_register(&disp_drv);    // 娉ㄥ唽鏄剧ず灞?

  static lv_indev_drv_t indev_drv1;
  lv_indev_drv_init(&indev_drv1);
  indev_drv1.type = LV_INDEV_TYPE_POINTER; // 璁剧疆涓鸿Е鎽稿睆绫诲瀷
  indev_drv1.read_cb = my_touchpad_read;   // 娉ㄥ唽鍥炶皟鍑芥暟
  lv_indev_drv_register(&indev_drv1);      // 娉ㄥ唽杈撳叆璁惧

  static lv_indev_drv_t indev_drv2;
  lv_indev_drv_init(&indev_drv2);
  indev_drv2.read_cb = keypad_read;                  // 娉ㄥ唽鍥炶皟鍑芥暟
  indev_drv2.type = LV_INDEV_TYPE_KEYPAD;            // 璁剧疆涓烘寜閿被鍨?
  indev_keypad = lv_indev_drv_register(&indev_drv2); // 娉ㄥ唽杈撳叆璁惧

  // 鍚姩瀹氭椂鍣ㄤ负lvgl鎻愪緵蹇冭烦鏃堕挓
  tim1 = timerBegin(0, 80, true);
  timerAttachInterrupt(tim1, tim1Interrupt, true);
  timerAlarmWrite(tim1, 1000, true);
  timerAlarmEnable(tim1);

  ledcWrite(1, 3);
  Wire.begin(18, 21);
  uint8_t devicesFound = 0;
  while (deviceNumber == UINT8_MAX)
  {
    devicesFound = INA.begin(10, 10000);
    Serial.println(INA.getDeviceName(devicesFound - 1));
    for (uint8_t i = 0; i < devicesFound; i++)
    {

      if (strcmp(INA.getDeviceName(i), "INA226") == 0)
      {
        deviceNumber = i;
        INA.reset(deviceNumber);

        break;
      }
    }
    if (deviceNumber == UINT8_MAX)
    {
      Serial.print(F("No INA found. Waiting 5s and retrying...\n"));
      delay(5000);
    }
  }
  Serial.print(F("Found INA at device number "));
  Serial.println(deviceNumber);
  Serial.println();
  INA.setAveraging(4, deviceNumber);
  INA.setBusConversion(8244, deviceNumber);            // 鏈€澶ц浆鎹㈡椂闂?.244ms
  INA.setShuntConversion(8244, deviceNumber);          // 鏈€澶ц浆鎹㈡椂闂?.244ms
  INA.setMode(INA_MODE_CONTINUOUS_BOTH, deviceNumber); // 杩炵画杞崲妯″紡

  Serial.println("ui_init start");

  ui_init(); // 鍒濆鍖杣i鐣岄潰
  Serial.println("ui_init done");
  Serial.println("Setup done");
  mcp4017_init();
  ledcWrite(1, 0);
}

void loop()
{
  // lvgl浠诲姟澶勭悊鍑芥暟
  lv_task_handler();
  // 鏇存柊铚傞福鍣ㄧ姸鎬?
  buzzer_update();
  // 鐢ㄦ埛浠诲姟澶勭悊鍑芥暟
  ina266_task();
  mcp4017_task();
  pwm_task();
  i2cscan_task();
  uart_helper_task();
  DSO_task();
  BluetoothSerial_task();
  FREcount_task();
  // 鍠傜湅闂ㄧ嫍锛岄槻姝㈢郴缁熼噸鍚?
  esp_task_wdt_reset();
}


