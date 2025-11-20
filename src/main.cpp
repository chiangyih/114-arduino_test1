/*
 * ============================================================================
 * 113 學年度 全國高級中等學校 工業類科學生技藝競賽
 * 電腦修護職類 第二站 — 個人電腦 USB / 藍牙介面卡製作及控制
 * 
 * MCU: ATmega328P (Arduino UNO)
 * 通訊: HC-05 Bluetooth SPP (9600bps)
 * 顯示: ST7735 TFT LCD (128x160, SPI)
 * RGB: WS2812 x 8 LEDs (D5)
 * 
 * 崗位號碼: 01
 * 藍牙名稱: ODD-01-0001
 * 
 * 功能列表:
 * F1 - CPU 運行指示燈閃爍 (D13)
 * F2 - TFT 顯示初始化畫面 (TCIVS/C201)
 * F3 - RGB Offline 選單 (Red/Green/Blue/Gradient)
 * F4 - CountDown 倒數計時 (00:00:10 -> 00:00:00)
 * F5 - 藍牙模組命名邏輯 (ODD/EVEN-XX-BBBB)
 * F6 - 藍牙通訊控制 (PING/CONNECT/DISCONNECT)
 * F7 - Connect to BLE 模式 (CPU Loading 顏色顯示)
 * F8 - EEPROM 寫入/讀取 (WRITE <DEC>)
 * 
 * 參考文件: FirmwareSpec.md
 * ============================================================================
 */

#include <Arduino.h>
#include <Engnin_comp_2025.h>
#include <EEPROM.h>

// ========== 腳位定義 ==========
#define LED_RED 13        // CPU 運行指示燈（D13）
#define WS2812_PIN 5      // WS2812 RGB LED（D5）- 修改為 D5
#define WS2812_COUNT 8    // WS2812 LED 數量

// TFT LCD 腳位定義（實際硬體配置 - 軟體 SPI）
// 注意：使用軟體 SPI 模式，可自訂 MOSI 和 SCK 腳位
// SDA (MOSI) 接於 A4
// SCL (SCK) 接於 A5
#define TFT_CS   10       // Chip Select（D10）
#define TFT_DC   8        // Data/Command（D8）
#define TFT_RST  9        // Reset（D9）
#define TFT_MOSI A4       // SDA / MOSI（A4）- 軟體 SPI
#define TFT_SCLK A5       // SCL / SCK（A5）- 軟體 SPI
#define TFT_BL   6        // 背光控制（D6）
// SPI 標準腳位（硬體 SPI）：
// MOSI (SDA) = D11（規格中的 D18）
// SCK (CLK) = D13（規格中的 D19）
// VCC = 5V
// GND = GND

// 按鍵腳位定義（根據 FirmwareSpec.md 硬體架構表）
#define KEY_UP     A0     // Up 按鍵（A0）
#define KEY_DOWN   A1     // Down 按鍵（A1）
#define KEY_ENTER  A2     // Enter 按鍵（A2）
#define KEY_RETURN A3     // Return 按鍵（A3）

// ========== 全域物件 ==========
// 使用軟體 SPI：Adafruit_ST7735(CS, DC, MOSI, SCLK, RST)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_NeoPixel strip(WS2812_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// ========== 全域變數定義 ==========

// ===== 選單系統相關 =====
// 選單狀態枚舉（五種狀態）
enum MenuState {
  MENU_MAIN,         // 主選單
  MENU_CONNECT_BLE,  // 藍牙連線選單
  MENU_RGB_OFFLINE,  // RGB 離線模式選單
  MENU_COUNTDOWN,    // 倒數計時選單
  MENU_EEPROM        // EEPROM 讀取選單
};

// RGB 模式枚舉（四種模式）
enum RGBMode {
  RGB_RED,       // 紅色模式（3 顆 LED）
  RGB_GREEN,     // 綠色模式（6 顆 LED）
  RGB_BLUE,      // 藍色模式（8 顆 LED）
  RGB_GRADIENT   // 漸層模式（8 顆 LED）
};

MenuState currentMenu = MENU_MAIN;  // 目前選單狀態
int menuIndex = 0;                  // 主選單索引（0-3）
int rgbModeIndex = 0;               // RGB 模式索引（0-3）
bool inSubMenu = false;             // 是否在子選單中

// ===== 倒數計時功能相關 =====
int countdownSeconds = 10;          // 倒數秒數（起始值 10）
bool countdownRunning = false;      // 倒數計時是否運行中
bool countdownPaused = false;       // 倒數計時是否暫停
unsigned long lastCountdownTime = 0; // 上次更新時間
bool countdownFinishAnimation = false; // 倒數完成後的閃爍動畫狀態
unsigned long countdownFinishLastToggle = 0; // 上次閃爍切換時間
uint8_t countdownFinishBlinkStep = 0;  // 閃爍步驟計數（6 步 = 3 次閃爍）
const unsigned long COUNTDOWN_FINISH_INTERVAL = 300; // 閃爍間隔（毫秒）

// ===== 藍牙通訊相關 =====
bool bleConnected = false;          // 藍牙連線狀態
String receivedData = "";           // 接收的資料緩衝區
unsigned long lastBleDataTime = 0;  // 最後一次收到藍牙資料的時間
const unsigned long BLE_TIMEOUT = 5000; // 藍牙逾時時間（5 秒）
const size_t BLE_BUFFER_MAX = 64;   // 藍牙命令最大長度

// ===== CPU 指示燈相關 =====
unsigned long lastLedTime = 0;      // 上次 LED 切換時間
bool ledState = false;              // LED 當前狀態（ON/OFF）

// ===== RGB LED 相關 =====
uint16_t hueValue = 0;              // 漸層色相值（0-65535）

// ===== EEPROM 資料儲存相關 =====
int eepromValue = 0;                // EEPROM 儲存的數值
bool eepromValid = false;           // EEPROM 資料是否有效

// ===== 按鍵防彈跳相關 =====
unsigned long lastKeyTime = 0;      // 上次按鍵觸發時間
const int KEY_DEBOUNCE = 200;       // 防彈跳時間（毫秒）

// ========== 函式宣告 ==========
void setupBluetooth();
void displayBootScreen();
void displayMainMenu();
void displaySubMenu();
void handleKeys();
void updateCPULed();
void updateRGBOffline();
void updateCountdown();
void handleBluetoothData();
void writeEEPROM(int value);
int readEEPROM();
void displayEEPROMValue();
void setWS2812Color(uint32_t color, int numLeds);
void setWS2812Gradient();
String getBinaryString(int number);

// ========== Timer1 中斷服務程式（用於倒數計時）==========
/**
 * @brief Timer1 溢位中斷服務常式 (ISR)
 * @note 此中斷每秒觸發一次，用於倒數計時功能
 * 
 * 功能說明：
 * 1. 重新載入 Timer1 計數器，維持 1Hz 中斷頻率
 * 2. 當倒數計時執行中且未暫停時，每秒遞減秒數
 * 3. 符合 FirmwareSpec.md F4 需求：從 10 秒倒數至 0 秒
 */
ISR(TIMER1_OVF_vect) {
  // 重新載入計數器，確保下次中斷在 1 秒後觸發
  TCNT1 = timer1_counter;
  
  // 倒數計時邏輯處理
  if (countdownRunning && !countdownPaused) {
    if (countdownSeconds > 0) {
      countdownSeconds--;  // 每秒遞減 1
    }
  }
}

// ========== Setup 函式（系統初始化）==========
/**
 * @brief 系統初始化函式，只在開機時執行一次
 * 
 * 初始化項目：
 * 1. 序列埠通訊（藍牙 HC-05）
 * 2. GPIO 腳位（LED、按鍵、TFT 背光）
 * 3. WS2812 RGB LED 燈條
 * 4. ST7735 TFT 顯示器
 * 5. 藍牙模組命名
 * 6. Timer1 中斷（倒數計時用）
 * 7. EEPROM 資料讀取
 */
void setup() {
  // ===== 1. 初始化序列埠通訊 =====
  // HC-05 藍牙模組使用 SPP 模式，鮑率 9600bps
  Serial.begin(9600);
  
  // ===== 2. 初始化 GPIO 腳位 =====
  // CPU 運行指示燈（紅色 LED）
  pinMode(LED_RED, OUTPUT);
  
  // TFT 背光控制腳位
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);   // 開啟背光（HIGH = 亮）
  
  // 按鍵初始化（使用內建上拉電阻，按下時為 LOW）
  pinMode(KEY_UP, INPUT_PULLUP);     // 向上按鍵
  pinMode(KEY_DOWN, INPUT_PULLUP);   // 向下按鍵
  pinMode(KEY_ENTER, INPUT_PULLUP);  // 確認按鍵
  pinMode(KEY_RETURN, INPUT_PULLUP); // 返回按鍵
  
  // ===== 3. 初始化 WS2812 RGB LED 燈條 =====
  strip.begin();                // 啟動 WS2812 控制
  strip.setBrightness(50);      // 設定亮度（範圍 0-255，50 約為 20%）
  strip.show();                 // 更新顯示（初始化為全部熄滅）
  
  // ===== 4. 初始化 ST7735 TFT 顯示器 =====
  delay(100);                   // 等待 TFT 電源穩定
  
  // 根據 TFT 模組背面的標籤顏色選擇初始化方式
  // 如果顯示異常（白屏、顏色錯誤），請嘗試其他選項
  tft.initR(INITR_BLACKTAB);    // 黑色標籤版本（1.8 吋 128×160 解析度）
  // tft.initR(INITR_GREENTAB);  // 綠色標籤版本（較常見，建議優先嘗試）
  // tft.initR(INITR_REDTAB);    // 紅色標籤版本
  // tft.initR(INITR_144GREENTAB); // 1.44 吋版本（144×128 解析度）
  
  delay(100);                   // 等待初始化完成
  
  // 設定螢幕方向（0-3 對應不同旋轉角度）
  tft.setRotation(1);           // 1 = 橫向顯示（最常用）
  
  // 清除螢幕為黑色背景
  tft.fillScreen(ST77XX_BLACK);
  delay(100);                   // 確保畫面清除完成
  
  // ===== 5. 設定藍牙模組名稱 =====
  // 根據崗位號碼的奇偶性命名（ODD 或 EVEN）
  setupBluetooth();
  
  // ===== 6. 顯示開機畫面 =====
  // 顯示「TCIVS」和「C201」文字
  displayBootScreen();
  
  // 符合 FirmwareSpec.md F2 需求：延遲 2 秒後進入選單
  delay(2000);
  displayMainMenu();
  
  // ===== 7. 初始化 Timer1 中斷 =====
  // 設定為 1Hz（每秒觸發一次），用於倒數計時功能
  // 計算公式：Timer1 計數值 = 65536 - (CPU頻率 / 預分頻 / 目標頻率)
  //          = 65536 - (16,000,000 / 256 / 1) = 65536 - 62,500 = 3,036
  timer_ini(3036);
  
  // ===== 8. 讀取 EEPROM 資料 =====
  // 讀取上次儲存的數值（用於 F8 功能）
  eepromValue = readEEPROM();
}

// ========== Loop 函式（主迴圈）==========
/**
 * @brief 主程式迴圈，不斷重複執行
 * 
 * 執行順序：
 * 1. 更新 CPU 指示燈閃爍（F1 功能）
 * 2. 處理按鍵輸入（選單切換、模式選擇）
 * 3. 處理藍牙資料（接收 PC 端命令）
 * 4. 根據目前選單狀態更新顯示
 */
void loop() {
  // ===== 1. 更新 CPU 運行指示燈 =====
  // F1: 紅色 LED 以 500ms 間隔閃爍
  updateCPULed();
  
  // ===== 2. 處理按鍵輸入 =====
  // 讀取四個按鍵狀態並執行對應動作（UP/DOWN/ENTER/RETURN）
  handleKeys();
  
  // ===== 3. 處理藍牙通訊 =====
  // 接收並解析來自 PC 端的命令（PING/CONNECT/DISCONNECT/WRITE/LOAD）
  handleBluetoothData();
  
  // ===== 4. 根據目前選單狀態更新顯示 =====
  switch (currentMenu) {
    case MENU_MAIN:
      // 主選單狀態：不需在 loop 中額外處理
      break;

    case MENU_RGB_OFFLINE:
      // RGB Offline 模式：更新 WS2812 LED 顯示
      if (inSubMenu) {
        updateRGBOffline();  // F3: 切換 Red/Green/Blue/Gradient 模式
      }
      break;
      
    case MENU_COUNTDOWN:
      // 倒數計時模式：更新倒數顯示
      if (inSubMenu) {
        updateCountdown();   // F4: 顯示 00:00:10 → 00:00:00
      }
      break;
      
    case MENU_CONNECT_BLE:
      // 藍牙連線模式：顯示連線狀態
      if (inSubMenu) {
        // F7: 根據 CPU Loading 顯示對應顏色
        // 檢查藍牙連線逾時（如果已連線但超過 5 秒沒收到資料）
        if (bleConnected && (millis() - lastBleDataTime > BLE_TIMEOUT)) {
          bleConnected = false;  // 設定為中斷連線
          
          // 更新 TFT 顯示為 Disconnected
          tft.fillRect(20, 40, 120, 20, ST77XX_BLACK);
          tft.setTextColor(ST77XX_RED);
          tft.setTextSize(2);
          tft.setCursor(20, 40);
          tft.print("Disconnect");
          
          // 清除 WS2812 顯示
          strip.clear();
          strip.show();
        }
      }
      break;
      
    case MENU_EEPROM:
      // EEPROM 讀取模式：顯示儲存的數值
      if (inSubMenu) {
        displayEEPROMValue();  // F8: 顯示 EEPROM 內容
        delay(50);             // 短暫延遲避免過度刷新
      }
      break;

    default:
      // 預設狀態：不做任何處理
      break;
  }
  
  // 移除阻塞式延遲，確保藍牙接收保持即時
}

// ========== 藍牙設定函式 ==========
/**
 * @brief 設定藍牙模組名稱
 * 
 * 命名規則（FirmwareSpec.md F5）：
 * - 格式：ODD-XX-BBBB 或 EVEN-XX-BBBB
 * - ODD/EVEN：根據崗位號碼的奇偶性判斷
 * - XX：崗位號碼（十進位，兩位數）
 * - BBBB：崗位號碼的二進位表示（4 位元）
 * 
 * 範例：
 * - 崗位 1 → ODD-01-0001
 * - 崗位 2 → EVEN-02-0010
 */
void setupBluetooth() {
  // 崗位號碼（實際使用時需根據比賽崗位修改）
  int stationNumber = 1;
  
  // 判斷奇偶數（使用模運算）
  String oddEven = (stationNumber % 2 == 1) ? "ODD" : "EVEN";
  
  // 將崗位號碼轉換為 4 位元二進位字串
  String binary = getBinaryString(stationNumber);
  
  // 組合完整藍牙名稱
  String btName = oddEven + "-0" + String(stationNumber) + "-" + binary;
  
  // 設定 HC-05 藍牙模組名稱
  // 注意：HC-05 需要先進入 AT 模式才能設定名稱
  // 進入方式：按住 HC-05 上的按鈕，然後上電
  // 以下為簡化處理，實際應用時需要完整的 AT 命令設定
  // Serial.println("AT+NAME=" + btName);
  
  delay(100);  // 等待設定完成
}

// ========== 顯示開機畫面 ==========
/**
 * @brief 顯示開機初始化畫面
 * 
 * FirmwareSpec.md F2 需求：
 * - 第一行顯示「TCIVS」
 * - 第二行顯示「C201」
 * - 黑色背景，白色文字
 * - 延遲 2 秒後進入主選單
 */
void displayBootScreen() {
  // 清除整個螢幕為黑色
  tft.fillScreen(ST77XX_BLACK);
  delay(200);  // 等待清除完成
  
  // 設定文字屬性
  tft.setTextColor(ST77XX_WHITE);  // 白色文字
  tft.setTextWrap(false);          // 關閉自動換行
  tft.setTextSize(3);              // 大字體（3 倍大小）
  
  // 第一行：顯示「TCIVS」
  tft.setCursor(25, 35);  // 設定游標位置（置中對齊）
  tft.print("TCIVS");
  
  // 第二行：顯示「C201」
  tft.setTextSize(2);      // 中等字體（2 倍大小）
  tft.setCursor(40, 70);   // 設定游標位置
  tft.print("C201");
}

// ========== 顯示主選單 ==========
/**
 * @brief 顯示主選單畫面
 * 
 * 選單項目（FirmwareSpec.md）：
 * 1. Connect to BLE  - 藍牙連線模式
 * 2. RGB Offline     - RGB LED 離線模式
 * 3. CountDown       - 倒數計時模式
 * 4. EEPROM          - EEPROM 讀取模式
 * 
 * 顯示特色：
 * - 選中項目使用藍色反白
 * - 選中項目前方顯示「>」符號
 * - 青色標題，白色選項文字
 */
void displayMainMenu() {
  // 清除螢幕
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);  // 小字體
  
  // 顯示選單標題「MENU」
  tft.setTextColor(ST77XX_CYAN);  // 青色標題
  tft.setCursor(55, 5);           // 置中位置
  tft.println("MENU");
  
  // 繪製標題下方的分隔線
  tft.drawFastHLine(0, 17, 160, ST77XX_WHITE);
  
  // 定義選單項目文字（對應四個功能）
  const char* menuItems[] = {
    "1.Connect to BLE",  // F6, F7: 藍牙連線與 CPU Loading 顯示
    "2.RGB Offline",     // F3: RGB LED 離線控制
    "3.CountDown",       // F4: 倒數計時功能
    "4.EEPROM"           // F8: EEPROM 讀取功能
  };
  
  // 逐一繪製選單項目
  for (int i = 0; i < 4; i++) {
    if (i == menuIndex) {
      // 選中的項目：藍色背景反白顯示
      tft.fillRect(0, 22 + i * 20, 160, 18, ST77XX_BLUE);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(5, 25 + i * 20);
      tft.print(">");  // 顯示箭頭指示符號
    } else {
      // 未選中的項目：白色文字
      tft.setTextColor(ST77XX_WHITE);
    }
    // 顯示選單項目文字
    tft.setCursor(15, 25 + i * 20);
    tft.println(menuItems[i]);
  }
}

// ========== 處理按鍵輸入 ==========
/**
 * @brief 讀取並處理四個按鍵的輸入
 * 
 * 按鍵配置：
 * - KEY_UP (A0)：向上移動選項
 * - KEY_DOWN (A1)：向下移動選項
 * - KEY_ENTER (A2)：確認進入/暫停繼續
 * - KEY_RETURN (A3)：返回上層選單
 * 
 * 防彈跳機制：
 * - 使用時間差判斷，避免重複觸發
 * - KEY_DEBOUNCE = 200ms
 */
void handleKeys() {
  unsigned long currentTime = millis();
  
  // 防彈跳處理：兩次按鍵間隔必須大於 200ms
  if (currentTime - lastKeyTime < KEY_DEBOUNCE) {
    return;  // 時間間隔太短，忽略此次按鍵
  }
  
  // ===== UP 鍵處理 =====
  if (digitalRead(KEY_UP) == LOW) {  // 按下時為 LOW（使用 INPUT_PULLUP）
    lastKeyTime = currentTime;        // 更新最後按鍵時間
    
    if (!inSubMenu) {
      // 主選單：向上移動選項（循環選擇）
      menuIndex = (menuIndex - 1 + 4) % 4;  // 避免負數，確保範圍 0-3
      displayMainMenu();  // 重新繪製選單
    } else if (currentMenu == MENU_RGB_OFFLINE) {
      // RGB Offline 子選單：切換上一個顏色模式
      rgbModeIndex = (rgbModeIndex - 1 + 4) % 4;  // 循環選擇 0-3
    }
  }
  
  // ===== DOWN 鍵處理 =====
  if (digitalRead(KEY_DOWN) == LOW) {
    lastKeyTime = currentTime;
    
    if (!inSubMenu) {
      // 主選單：向下移動選項（循環選擇）
      menuIndex = (menuIndex + 1) % 4;  // 範圍 0-3
      displayMainMenu();
    } else if (currentMenu == MENU_RGB_OFFLINE) {
      // RGB Offline 子選單：切換下一個顏色模式
      rgbModeIndex = (rgbModeIndex + 1) % 4;
    }
  }
  
  // ===== ENTER 鍵處理 =====
  if (digitalRead(KEY_ENTER) == LOW) {
    lastKeyTime = currentTime;
    
    if (!inSubMenu) {
      // 主選單狀態：進入選中的子選單
      inSubMenu = true;
      
      // 正確對應 menuIndex 到 MenuState
      // 修正：直接轉型會造成選單錯亂，需要明確對應
      // menuIndex: 0=Connect BLE, 1=RGB Offline, 2=CountDown, 3=EEPROM
      switch (menuIndex) {
        case 0:
          currentMenu = MENU_CONNECT_BLE;
          break;
        case 1:
          currentMenu = MENU_RGB_OFFLINE;
          break;
        case 2:
          currentMenu = MENU_COUNTDOWN;
          break;
        case 3:
          currentMenu = MENU_EEPROM;
          break;
      }
      
      // 根據選擇的選單顯示對應畫面
      switch (currentMenu) {
        case MENU_CONNECT_BLE:
          // F6, F7: 顯示藍牙連線畫面
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextColor(ST77XX_CYAN);
          tft.setTextSize(1);
          tft.setCursor(25, 5);
          tft.print("Connect to BLE");
          tft.drawFastHLine(0, 17, 160, ST77XX_WHITE);
          
          // 顯示連線狀態
          tft.setTextSize(2);
          tft.setCursor(20, 40);
          if (bleConnected) {
            tft.setTextColor(ST77XX_GREEN);
            tft.print("Connected");
          } else {
            tft.setTextColor(ST77XX_RED);
            tft.print("Disconnect");
          }
          
          // 顯示說明文字
          tft.setTextColor(ST77XX_WHITE);
          tft.setTextSize(1);
          tft.setCursor(5, 70);
          tft.print("PC send command:");
          tft.setCursor(5, 82);
          tft.print("CONNECT or PING");
          tft.setCursor(5, 94);
          tft.print("or any data...");
          
          tft.setCursor(5, 112);
          tft.print("Return:Exit");
          break;
          
        case MENU_RGB_OFFLINE:
          // F3: RGB Offline 模式初始化
          rgbModeIndex = 0;  // 重置為第一個模式（Red）
          tft.fillScreen(ST77XX_BLACK);
          break;
          
        case MENU_COUNTDOWN:
          // F4: 倒數計時模式初始化
          countdownSeconds = 10;      // 起始時間 10 秒
          countdownRunning = true;     // 開始倒數
          countdownPaused = false;     // 非暫停狀態
          tft.fillScreen(ST77XX_BLACK);
          break;
          
        case MENU_EEPROM:
          // F8: 顯示 EEPROM 內容
          displayEEPROMValue();
          break;
      }
    } else if (currentMenu == MENU_COUNTDOWN) {
      // 倒數計時子選單：ENTER 鍵切換暫停/繼續
      countdownPaused = !countdownPaused;  // 反轉暫停狀態
    }
  }
  
  // ===== RETURN 鍵處理 =====
  if (digitalRead(KEY_RETURN) == LOW) {
    lastKeyTime = currentTime;
    
    if (inSubMenu) {
      // 子選單狀態：返回主選單
      inSubMenu = false;
      countdownRunning = false;  // 停止倒數計時
      strip.clear();             // 清除 WS2812 LED
      strip.show();
      displayMainMenu();         // 顯示主選單
    }
  }
}

// ========== 更新 CPU 運行指示燈 ==========
/**
 * @brief 更新 CPU 運行指示燈閃爍狀態
 * 
 * FirmwareSpec.md F1 需求：
 * - 紅色 LED (D13) 以 500ms 間隔閃爍
 * - 持續運行，表示 MCU 正常工作
 */
void updateCPULed() {
  unsigned long currentTime = millis();
  
  // 每 500ms 切換一次 LED 狀態
  if (currentTime - lastLedTime > 500) {
    lastLedTime = currentTime;    // 更新時間戳記
    ledState = !ledState;         // 反轉 LED 狀態
    digitalWrite(LED_RED, ledState ? HIGH : LOW);  // 輸出到 D13
  }
}

// ========== 更新 RGB Offline 模式顯示 ==========
/**
 * @brief 更新 RGB Offline 模式的 TFT 顯示和 LED 控制
 * 
 * FirmwareSpec.md F3 需求：
 * 四種模式：
 * - Red: 3 顆 LED 閃爍紅色
 * - Green: 6 顆 LED 閃爍綠色
 * - Blue: 8 顆 LED 閃爍藍色
 * - Gradient: 8 顆 LED 顯示漸層色彩
 */
void updateRGBOffline() {
  static int lastRGBMode = -1;
  
  // 只在模式改變時更新 TFT 顯示（避免重複繪製）
  if (lastRGBMode != rgbModeIndex) {
    lastRGBMode = rgbModeIndex;
    
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(35, 5);
    tft.print("RGB Offline");
    
    tft.drawFastHLine(0, 17, 160, ST77XX_WHITE);
    
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    
    switch (rgbModeIndex) {
      case RGB_RED:
        tft.setCursor(10, 40);
        tft.print("Red");
        tft.setTextSize(1);
        tft.setCursor(10, 65);
        tft.print("3 LEDs Blink");
        break;
        
      case RGB_GREEN:
        tft.setCursor(10, 40);
        tft.print("Green");
        tft.setTextSize(1);
        tft.setCursor(10, 65);
        tft.print("6 LEDs Blink");
        break;
        
      case RGB_BLUE:
        tft.setCursor(10, 40);
        tft.print("Blue");
        tft.setTextSize(1);
        tft.setCursor(10, 65);
        tft.print("8 LEDs Blink");
        break;
        
      case RGB_GRADIENT:
        tft.setCursor(10, 40);
        tft.print("Gradient");
        tft.setTextSize(1);
        tft.setCursor(10, 65);
        tft.print("RGB Cycle");
        break;
    }
    
    // 操作提示
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.setCursor(5, 100);
    tft.print("Up/Down:Change Mode");
    tft.setCursor(5, 112);
    tft.print("Return:Exit");
  }
  
  // 根據模式控制 WS2812
  switch (rgbModeIndex) {
    case RGB_RED:
      setWS2812Color(strip.Color(255, 0, 0), 3);
      break;
      
    case RGB_GREEN:
      setWS2812Color(strip.Color(0, 255, 0), 6);
      break;
      
    case RGB_BLUE:
      setWS2812Color(strip.Color(0, 0, 255), 8);
      break;
      
    case RGB_GRADIENT:
      setWS2812Gradient();
      break;
  }
  
  delay(50);
}

// ========== 設定 WS2812 顏色 ==========
void setWS2812Color(uint32_t color, int numLeds) {
  static unsigned long lastBlink = 0;
  static bool blinkState = false;
  
  if (millis() - lastBlink > 500) {
    lastBlink = millis();
    blinkState = !blinkState;
  }
  
  for (int i = 0; i < WS2812_COUNT; i++) {
    if (i < numLeds && blinkState) {
      strip.setPixelColor(i, color);
    } else {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

// ========== 設定 WS2812 漸層色 ==========
void setWS2812Gradient() {
  for (int i = 0; i < WS2812_COUNT; i++) {
    uint16_t hue = (hueValue + (i * 65536L / WS2812_COUNT)) % 65536;
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
  }
  strip.show();
  hueValue += 256;  // 緩慢變色
}

// ========== 更新倒數計時 ==========
void updateCountdown() {
  static int lastDisplaySeconds = -1;
  
  // 只在秒數改變時更新顯示
  if (lastDisplaySeconds != countdownSeconds) {
    lastDisplaySeconds = countdownSeconds;
    
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    
    // 標題
    tft.setCursor(40, 5);
    tft.print("CountDown");
    
    // 顯示時間（格式：00:00:10）
    tft.setTextSize(3);
    int minutes = countdownSeconds / 60;
    int seconds = countdownSeconds % 60;
    
    tft.setCursor(20, 35);
    tft.print("00:");
    if (minutes < 10) tft.print("0");
    tft.print(minutes);
    tft.print(":");
    if (seconds < 10) tft.print("0");
    tft.print(seconds);
    
    // 顯示狀態
    tft.setTextSize(1);
    tft.setCursor(50, 75);
    if (countdownPaused) {
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("PAUSED");
    } else {
      tft.setTextColor(ST77XX_GREEN);
      tft.print("RUNNING");
    }
    
    // 操作提示
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(5, 100);
    tft.print("Enter:Pause/Resume");
    tft.setCursor(5, 112);
    tft.print("Return:Exit");
  }
  
  // 倒數結束時啟動非阻塞閃爍動畫（根據 FirmwareSpec.md）
  if (countdownSeconds == 0 && countdownRunning) {
    countdownRunning = false;
    countdownFinishAnimation = true;
    countdownFinishBlinkStep = 0;
    countdownFinishLastToggle = millis();
    
    // 更新 TFT 顯示完成訊息
    tft.fillRect(0, 75, 160, 20, ST77XX_BLACK);
    tft.setTextColor(ST77XX_MAGENTA);
    tft.setTextSize(2);
    tft.setCursor(30, 75);
    tft.print("FINISH!");
  }
  
  if (countdownFinishAnimation) {
    unsigned long now = millis();
    if (now - countdownFinishLastToggle >= COUNTDOWN_FINISH_INTERVAL) {
      countdownFinishLastToggle = now;
      bool ledOn = (countdownFinishBlinkStep % 2 == 0);
      for (int j = 0; j < WS2812_COUNT; j++) {
        strip.setPixelColor(j, ledOn ? strip.Color(255, 105, 180) : 0);
      }
      strip.show();
      countdownFinishBlinkStep++;
      if (countdownFinishBlinkStep >= 6) {
        for (int j = 0; j < WS2812_COUNT; j++) {
          strip.setPixelColor(j, strip.Color(255, 105, 180));
        }
        strip.show();
        countdownFinishAnimation = false;
      }
    }
  }
}

// ========== 處理藍牙資料 ==========
/**
 * @brief 處理藍牙序列埠接收的資料
 * 
 * 支援的命令（FirmwareSpec.md F6）：
 * - PING：心跳確認
 * - CONNECT：建立連線
 * - DISCONNECT：中斷連線
 * - WRITE <DEC>：寫入 EEPROM
 * - LOAD <VAL>：更新 CPU Loading 顏色
 */
void handleBluetoothData() {
  // 只要收到任何資料，就視為藍牙已連線（自動偵測連線）
  if (Serial.available() > 0) {
    // 更新最後收到資料的時間（用於逾時檢測）
    lastBleDataTime = millis();
    
    if (!bleConnected) {
      bleConnected = true;
      
      // 如果在 BLE 選單中，立即更新顯示
      if (currentMenu == MENU_CONNECT_BLE && inSubMenu) {
        tft.fillRect(20, 40, 120, 20, ST77XX_BLACK);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(2);
        tft.setCursor(20, 40);
        tft.print("Connected");
      }
    }
  }
  
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (receivedData.length() > 0) {
        // 處理接收到的命令
        receivedData.trim(); // 收到的資料去除空白、換行 等(.trim的屬性是指刪除字串前後的空白字元)
        
        // // 除錯輸出：顯示接收到的藍牙資料
        // Serial.println("Waiting for BLE data...");
        Serial.print("BLE RX: ");
        Serial.println(receivedData);
        
        // WRITE 命令：寫入 EEPROM（格式：WRITE <DEC>）
        if (receivedData.startsWith("WRITE ")) {
          int value = receivedData.substring(6).toInt();
          
          // 根據 FirmwareSpec.md：接受四位二進位數值（由 PC 端轉十進位後傳送）
          if (value >= 0 && value <= 255) {
            writeEEPROM(value);
            Serial.println("ACK");
            
            // 更新顯示（如果在 EEPROM 選單中）
            if (currentMenu == MENU_EEPROM && inSubMenu) {
              displayEEPROMValue();
            }
          } else {
            Serial.println("ERR");
          }
        }
        // LOAD 命令：更新 WS2812 顏色（格式：LOAD <VAL>）
        else if (receivedData.startsWith("LOAD")) {
          // 提取數值：移除 "LOAD" 後 trim 空格再轉換
          String valueStr = receivedData.substring(4);
          valueStr.trim();  // 去除前後空格
          int cpuLoad = valueStr.toInt();
          
          Serial.print("CPU Load: ");
          Serial.println(cpuLoad);
          // 根據 CPU Loading 百分比顯示不同顏色（依 Arduino_WS2812_Integration_Guide.md 規範）
          uint32_t color;
          if (cpuLoad <= 50) {
            color = strip.Color(0, 255, 0);      // 綠色：0-50% (正常負載)
          } else if (cpuLoad <= 84) {
            color = strip.Color(255, 255, 0);    // 黃色：51-84% (中度負載)
          } else {
            color = strip.Color(255, 0, 0);      // 紅色：85-100% (高負載)
          }
          
          // 設定所有 WS2812 LED
          for (int i = 0; i < WS2812_COUNT; i++) {
            strip.setPixelColor(i, color);
          }
          strip.show();
          
          Serial.println("ACK");
        }
        // PING 命令：心跳確認（格式：PING -> ACK）
        else if (receivedData == "PING") {
          bleConnected = true;
          Serial.println("ACK");
        }
        // CONNECT 命令：建立連線
        else if (receivedData == "CONNECT") {
          bleConnected = true;
          Serial.println("ACK");
          
          // 如果在 BLE 選單中，更新顯示
          if (currentMenu == MENU_CONNECT_BLE && inSubMenu) {
            tft.fillRect(20, 40, 120, 20, ST77XX_BLACK);
            tft.setTextColor(ST77XX_GREEN);
            tft.setTextSize(2);
            tft.setCursor(20, 40);
            tft.print("Connected");
          }
        }
        // DISCONNECT 命令：中斷連線
        else if (receivedData == "DISCONNECT") {
          bleConnected = false;
          Serial.println("ACK");
          
          // 如果在 BLE 選單中，更新顯示
          if (currentMenu == MENU_CONNECT_BLE && inSubMenu) {
            tft.fillRect(20, 40, 120, 20, ST77XX_BLACK);
            tft.setTextColor(ST77XX_RED);
            tft.setTextSize(2);
            tft.setCursor(20, 40);
            tft.print("Disconnect");
          }
          
          // 清除 WS2812 顯示
          strip.clear();
          strip.show();
        }
        
        receivedData = "";
      }
    } else {
      if (receivedData.length() >= BLE_BUFFER_MAX - 1) {
        receivedData = "";
        Serial.println("ERR");
      } else {
        receivedData += c;
      }
    }
  }
}

// ========== EEPROM 寫入 ==========
// 根據 FirmwareSpec.md：接受四位二進位數值（由 PC 端轉十進位後傳送）
// 四位二進位範圍：0000-1111 (0-15)，但規格允許更大範圍（0-255）
void writeEEPROM(int value) {
  // 檢查數值範圍
  if (value >= 0 && value <= 255) {
    EEPROM.write(0, value);
    eepromValue = value;
    eepromValid = true;
  } else {
    // 若數值錯誤，維持前次正確值不更新
    eepromValid = false;
  }
}

// ========== EEPROM 讀取 ==========
// 從 EEPROM 地址 0 讀取儲存的數值
int readEEPROM() {
  int value = EEPROM.read(0);
  // 驗證讀取的數值
  if (value >= 0 && value <= 255) {
    eepromValid = true;
    return value;
  }
  // 若讀取失敗，返回 0 並標記為無效
  eepromValid = false;
  return 0;
}

// ========== 顯示 EEPROM 數值 ==========
void displayEEPROMValue() {
  static bool displayUpdated = false;
  
  if (!displayUpdated) {
    displayUpdated = true;
    
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(50, 5);
    tft.print("EEPROM");
    tft.drawFastHLine(0, 17, 160, ST77XX_WHITE);
    
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 30);
    tft.print("Stored Value:");
    
    tft.setTextSize(3);
    tft.setCursor(40, 55);
    
    if (eepromValid) {
      tft.setTextColor(ST77XX_GREEN);
      tft.print(eepromValue);
      
      // 顯示十進位說明
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_YELLOW);
      tft.setCursor(10, 90);
      tft.print("(Decimal Value)");
    } else {
      tft.setTextColor(ST77XX_RED);
      tft.print("ERR");
      
      tft.setTextSize(1);
      tft.setCursor(10, 90);
      tft.print("Error: EEPROM");
    }
    
    // 操作提示
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(5, 112);
    tft.print("Return:Exit");
  }
  
  // 離開選單時重置標記
  if (!inSubMenu) {
    displayUpdated = false;
  }
}

// ========== 取得二進位字串 ==========
String getBinaryString(int number) {
  String binary = "";
  for (int i = 3; i >= 0; i--) {
    binary += ((number >> i) & 1) ? "1" : "0";
  }
  return binary;
}
