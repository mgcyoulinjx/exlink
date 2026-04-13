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
#include <Preferences.h>

#define I2C_SDA 18 // 触摸SDA引脚
#define I2C_SCL 21 // 触摸SCL引脚
#define RST_N_PIN 16
#define INT_N_PIN 18
#define RIGHT_PIN 38
#define LEFT_PIN 39
#define PUSH_PIN 40
#define PCNT_UNIT PCNT_UNIT_0                                         // 使用PCNT单元0
#define SHORT_PRESS_THRESHOLD 10                                      // 定义短按时间阈值
#define DOUBLE_CLICK_THRESHOLD 450                                    // 定义双击时间阈值
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"           // 服务UUID
#define CHARACTERISTIC_UUID_RX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // 接收特征UUID
#define CHARACTERISTIC_UUID_TX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // 鍙戦€佺壒寰乁UID

int adcValues[SAMPLE_COUNT];                                   // 存储ADC采样数值
char maxValueStr[20], minValueStr[20], peakToPeakValueStr[20]; // 存储最大最小峰峰值

volatile int pulseCount = 0;
float frequency = 0.0;
char freqencyStr[20];
int16_t count = 0;

unsigned long previousMillis = 0; // 记录上次读取计数的时间

const long interval = 125; // 设置脉冲读取间隔为125毫秒

INA_Class INA;
int8_t request_index = 0;
char voltageStr[20], currentStr[20], powerStr[20], mAHStr[20]; // 存储ina226测量数据
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
int buzzer_volume_level = 3;
static Preferences app_preferences;
static bool preferences_ready = false;
static const char *PREF_NAMESPACE = "exlink";
static const char *PREF_KEY_BUZZER_VOLUME = "buzzer_vol";

int pwm_Freq = 0;
int pwm_Duty = 0;

int new_pwm_Freq = 0;
int new_pwm_Duty = 0;

int mcp4107_value;
long currentBaudRate = 115200; // 定义初始波特率
lv_indev_t *indev_keypad;
bool i2cscan_requested = false;

static void sync_i2c_scan_button_visual()
{
  if (i2con == nullptr)
  {
    return;
  }

  lv_obj_set_style_bg_color(
      i2con,
      lv_color_hex(i2cscan_flag ? 0x16C47F : 0xFF7A00),
      0);
}
byte error, address;
int nDevices;
// 在这里设置屏幕尺寸
static const uint32_t screenWidth = TFT_WIDTH;
static const uint32_t screenHeight = TFT_HEIGHT;
// lvgl显示存储数组
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 2];
CST816T cst816t(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN);
hw_timer_t *tim1 = NULL;
TFT_eSPI tft = TFT_eSPI();

BLEServer *pServer = NULL;            // BLEServer指针pServer
BLECharacteristic *pTxCharacteristic; // BLECharacteristic指针 pTxCharacteristic
BLECharacteristic *pRxCharacteristic; // BLECharacteristic指针 pRxCharacteristic
bool deviceConnected = false;         // 本次连接状态
bool oldDeviceConnected = false;      // 上次连接状态
uint8_t txValue = 0;
static char pending_ble_rx_text[512];
static volatile size_t pending_ble_rx_len = 0;
static char uart_rx_chunk[128];
static char ble_tx_chunk[128];
static const long kSupportedBaudRates[] = {
    1200, 2400, 4800, 9600, 19200, 43000, 76800,
    115200, 128000, 230400, 256000, 460800, 921600};

static void append_wireless_uart_text(const char *text, bool add_newline)
{
  if (!wireless_uart_extarea || !text)
  {
    return;
  }

  if (strlen(lv_textarea_get_text(wireless_uart_extarea)) > 768)
  {
    lv_textarea_set_text(wireless_uart_extarea, "");
  }

  lv_textarea_add_text(wireless_uart_extarea, text);
  if (add_newline)
  {
    lv_textarea_add_text(wireless_uart_extarea, "\n");
  }
}

static void append_uart_helper_text(const char *text)
{
  if (!uart_extarea || !text || text[0] == '\0')
  {
    return;
  }

  if (strlen(lv_textarea_get_text(uart_extarea)) > 768)
  {
    lv_textarea_set_text(uart_extarea, "");
  }

  lv_textarea_add_text(uart_extarea, text);
}

static size_t read_serial_chunk(char *buffer, size_t buffer_size)
{
  if (!buffer || buffer_size < 2)
  {
    return 0;
  }

  size_t count = 0;
  while (count < buffer_size - 1 && Serial.available() > 0)
  {
    buffer[count++] = (char)Serial.read();
  }
  buffer[count] = '\0';
  return count;
}

static long get_selected_baud_rate(lv_obj_t *dropdown)
{
  if (!dropdown)
  {
    return currentBaudRate;
  }

  uint16_t selected_index = lv_dropdown_get_selected(dropdown);
  size_t baud_count = sizeof(kSupportedBaudRates) / sizeof(kSupportedBaudRates[0]);
  if (selected_index >= baud_count)
  {
    selected_index = 7;
  }

  return kSupportedBaudRates[selected_index];
}

static void update_serial_baud_rate(lv_obj_t *dropdown)
{
  long newBaudRate = get_selected_baud_rate(dropdown);
  if (newBaudRate != currentBaudRate)
  {
    Serial.end();
    Serial.begin(newBaudRate);
    currentBaudRate = newBaudRate;
  }
}

class MyServerCallbacks : public BLEServerCallbacks // BLE连接回调函数
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

class MyCallbacks : public BLECharacteristicCallbacks // BLE接收回调函数
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.write((const uint8_t *)rxValue.data(), rxValue.length());

      size_t available_space = sizeof(pending_ble_rx_text) - pending_ble_rx_len - 1;
      if (available_space == 0)
      {
        pending_ble_rx_len = 0;
        pending_ble_rx_text[0] = '\0';
        available_space = sizeof(pending_ble_rx_text) - 1;
      }

      size_t copy_len = rxValue.length();
      if (copy_len > available_space)
      {
        copy_len = available_space;
      }

      memcpy(pending_ble_rx_text + pending_ble_rx_len, rxValue.data(), copy_len);
      pending_ble_rx_len += copy_len;

      if (pending_ble_rx_len < sizeof(pending_ble_rx_text) - 1 && (copy_len == 0 || pending_ble_rx_text[pending_ble_rx_len - 1] != '\n'))
      {
        pending_ble_rx_text[pending_ble_rx_len++] = '\n';
      }

      pending_ble_rx_text[pending_ble_rx_len] = '\0';
    }
  }
};

void mcp4017_init() // 数字电位器初始化
{
  Wire.beginTransmission(0x2F);
  Wire.write(0x56);
  Wire.endTransmission();
}

void mcp4017_task() // 数字电位器任务
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

// 非阻塞蜂鸣器控制
static uint32_t buzzer_start_time = 0;
static bool buzzer_active = false;
#define BUZZER_DURATION 50  // 蜂鸣器持续时间(ms)

extern "C" void persist_buzzer_volume_level(int level)
{
  int clamped_level = constrain(level, 1, 4);
  buzzer_volume_level = clamped_level;
  if (preferences_ready)
  {
    app_preferences.putUChar(PREF_KEY_BUZZER_VOLUME, (uint8_t)clamped_level);
  }
}

static void load_buzzer_volume_level()
{
  if (!preferences_ready)
  {
    return;
  }

  int stored_level = app_preferences.getUChar(PREF_KEY_BUZZER_VOLUME, 3);
  if (stored_level < 1 || stored_level > 4)
  {
    stored_level = 3;
  }
  buzzer_volume_level = stored_level;
}

void buzzer_beep() // 触发蜂鸣器（非阻塞）
{
  if (!buzzer_active)
  {
    ledcWrite(1, buzzer_volume_level);
    buzzer_active = true;
    buzzer_start_time = millis();
  }
}

void buzzer_update() // 更新蜂鸣器状态（在loop中调用）
{
  if (buzzer_active && (millis() - buzzer_start_time >= BUZZER_DURATION))
  {
    ledcWrite(1, 0);
    buzzer_active = false;
  }
}

void pwm_task() // pwm输出任务
{

  if (pwm_flag == 1)
  {

    const char *freq_str = lv_textarea_get_text(fre);
    const char *duty_str = lv_textarea_get_text(duty);
    pwm_Freq = atoi(freq_str);
    pwm_Duty = constrain(atoi(duty_str), 0, 100); // 限制占空比在 0-100 之间
    pwm_Duty = map(pwm_Duty, 0, 100, 0, 255);

    if (pwm_Freq != new_pwm_Freq || pwm_Duty != new_pwm_Duty)
    {
      ledcAttachPin(5, 2);       // 将引脚连接到通道2
      ledcSetup(2, pwm_Freq, 8); // 设置通道2为8位分辨率
      ledcWrite(2, pwm_Duty);    // 通道2输出
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

void i2cscan_task() // I2C扫描任务
{
  if (i2cscan_flag == 1 && i2cscan_requested)
  {
    Wire.begin(6, 7);
    lv_textarea_add_text(i2c_extarea, "Scanning...\n");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
      if (i2cscan_flag == 0)
      {
        lv_textarea_add_text(i2c_extarea, "Scan cancelled\n");
        break;
      }

      Wire1.begin(6, 7);
      Wire1.beginTransmission(address); // 从指定的地址开始向I2C从设备进行传输
      error = Wire1.endTransmission();  // 停止与从机的数据传输
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

    i2cscan_requested = false;
    sync_i2c_scan_button_visual();
  }
}

void uart_helper_task() // 串口助手任务
{

  if (uart_helper_flag == 1)
  {
    update_serial_baud_rate(uart_list);
    size_t bytes_read = read_serial_chunk(uart_rx_chunk, sizeof(uart_rx_chunk));
    if (bytes_read > 0)
    {
      append_uart_helper_text(uart_rx_chunk);
    }
  }
}

void DSO_task() // 简易示波器任务
{

  if (DSO_flag == 1)
  {
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
      adcValues[i] = analogRead(4); // Read ADC values
      lv_chart_set_next_value(DSO_chart, DSO_ser, adcValues[i]);
    }

    float maxValue = 0;
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

    peakToPeakValue = maxValue - minValue; // 计算峰峰值

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

void ina266_task() // 功率监测任务
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

void BluetoothSerial_task() // 无线串口任务（基于BLE实现）
{
  if (BluetoothSerial_flag == 1)
  {
    BLEDevice::init("Exlink"); // 创建丢个BLE设备

    pServer = BLEDevice::createServer();            // 创建一个BLE服务器
    pServer->setCallbacks(new MyServerCallbacks()); // 设置回调

    BLEService *pService = pServer->createService(SERVICE_UUID); // 创建蓝牙服务

    // 创建发送特征，添加描述符，设置通知权限
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    // 创建接收特征，设置回调函数，设置可写权限
    pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();                  // 启动服务
    pServer->getAdvertising()->start(); // 开始广播
    BluetoothSerial_flag = 2;
  }
  if (BluetoothSerial_flag == 2)
  {
    update_serial_baud_rate(wireless_uart_list);

    if (deviceConnected) // 连接时执行串口转发
    {
      if (pending_ble_rx_len > 0)
      {
        append_wireless_uart_text(pending_ble_rx_text, false);
        pending_ble_rx_len = 0;
        pending_ble_rx_text[0] = '\0';
      }

      size_t bytes_read = read_serial_chunk(ble_tx_chunk, sizeof(ble_tx_chunk));
      if (bytes_read > 0)
      {
        pTxCharacteristic->setValue((uint8_t *)ble_tx_chunk, bytes_read);
        pTxCharacteristic->notify();
        append_wireless_uart_text(ble_tx_chunk, false);
      }
    }

    if (!deviceConnected && oldDeviceConnected) // 未连接时执行蓝牙广播
    {
      pServer->startAdvertising();
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) // 更新蓝牙状态
    {
      oldDeviceConnected = deviceConnected;
    }
  }
}

void FREcount_task() // 数字频率计任务
{
  if (FREcount_flag == 1)
  {
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = 5,                // 输入引脚
        .ctrl_gpio_num = PCNT_PIN_NOT_USED, // 不使用控制引脚
        .lctrl_mode = PCNT_MODE_KEEP,       // 脉冲计数模式
        .hctrl_mode = PCNT_MODE_KEEP,       // 禁用高电平计数
        .pos_mode = PCNT_COUNT_INC,         // 正脉冲计数
        .neg_mode = PCNT_COUNT_DIS,         // 禁用负脉冲计数
        .unit = PCNT_UNIT,                  // 使用的PCNT单元
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config); // 配置PCNT单元
    pcnt_counter_clear(PCNT_UNIT);  // 启动计数
    FREcount_flag = 2;
  }
  if (FREcount_flag == 2)
  {
    unsigned long currentMillis = millis();
    pcnt_get_counter_value(PCNT_UNIT, &count);
    unsigned long currentinterval = currentMillis - previousMillis;
    if (currentinterval >= interval) // 如果达到读取间隔
    {

      //  计算频率（假设每次读取的时间为1秒）
      //  unsigned long frequency = count/currentinterval*1000; // 频率 = 计数值（赫兹）
      sprintf(freqencyStr, "%d", count * 8); // 清零计数
      pcnt_counter_clear(PCNT_UNIT);         // 更新数据
      previousMillis = currentMillis;        // 更新上次读取计数的时间
    }
  }
}

// 屏幕打点函数
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

// 触摸屏回调函数
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

// 定时器中断服务函数
void tim1Interrupt()
{
  lv_tick_inc(1);
}

// lvgl任务处理函数
void lvgl_handler(void *pvParameters)
{
  while (1)
  {
    lv_timer_handler(); /* let the GUI do its work */
    esp_task_wdt_reset(); /* 喂看门狗，防止重启*/
    delay(5);
  }
}

void setup()
{
  Serial.begin(115200);
  preferences_ready = app_preferences.begin(PREF_NAMESPACE, false);
  load_buzzer_volume_level();
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
  tft.setRotation(3); // 设置显示方向
  // cst816t.begin();    // 初始化触摸屏

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 2);
  static lv_disp_drv_t disp_drv; // 初始化显示器

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = tft.width();
  disp_drv.ver_res = tft.height();
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.sw_rotate = 0;             // 屏幕镜像
  disp_drv.rotated = LV_DISP_ROT_NONE; // 屏幕旋转
  lv_disp_drv_register(&disp_drv);    // 注册显示屏

  static lv_indev_drv_t indev_drv1;
  lv_indev_drv_init(&indev_drv1);
  indev_drv1.type = LV_INDEV_TYPE_POINTER; // 设置为触摸屏类型
  indev_drv1.read_cb = my_touchpad_read;   // 注册回调函数
  lv_indev_drv_register(&indev_drv1);      // 注册输入设备

  static lv_indev_drv_t indev_drv2;
  lv_indev_drv_init(&indev_drv2);
  indev_drv2.read_cb = keypad_read;                  // 注册回调函数
  indev_drv2.type = LV_INDEV_TYPE_KEYPAD;            // 设置为按键类型
  indev_keypad = lv_indev_drv_register(&indev_drv2); // 注册输入设备

  // 启动定时器为lvgl提供心跳时钟
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
  INA.setBusConversion(8244, deviceNumber);            // 最大转换时间8.244ms
  INA.setShuntConversion(8244, deviceNumber);          // 最大转换时间8.244ms
  INA.setMode(INA_MODE_CONTINUOUS_BOTH, deviceNumber); // 连续转换模式

  Serial.println("ui_init start");

  ui_init(); // 初始化ui界面
  Serial.println("ui_init done");
  Serial.println("Setup done");
  mcp4017_init();
  ledcWrite(1, 0);
}

void loop()
{
  // lvgl任务处理函数
  lv_task_handler();
  // 更新蜂鸣器状态
  buzzer_update();
  // 用户任务处理函数
  ina266_task();
  mcp4017_task();
  pwm_task();
  i2cscan_task();
  uart_helper_task();
  DSO_task();
  BluetoothSerial_task();
  FREcount_task();
  // 喂看门狗，防止系统重启
  esp_task_wdt_reset();
}

