/*
 * ============================================================================
 * Engnin_comp_2025.h
 * 113 學年度 全國高級中等學校 工業類科學生技藝競賽
 * 電腦修護職類 第二站 專用標頭檔
 * 
 * 功能：
 * 1. 引入所需的 Adafruit 函式庫（NeoPixel、GFX、ST7735）
 * 2. 引入 FreeMono 字型（9pt, 12pt, 18pt, 24pt）
 * 3. 提供按鍵讀取巨集（keyB, keyC, keyD）
 * 4. 提供 Timer1 初始化函式（timer_ini）
 * 5. 提供 24 位元轉 16 位元顏色轉換函式（convert24to16）
 * ============================================================================
 */

// ===== 引入必要的函式庫 =====
#include <Adafruit_NeoPixel.h>    // WS2812 RGB LED 控制函式庫
#include <Adafruit_GFX.h>          // Adafruit 圖形函式庫（基礎繪圖函式）
#include <Adafruit_ST7735.h>       // ST7735 TFT LCD 顯示器驅動函式庫
#include <Fonts/FreeMono9pt7b.h>   // 字型 9pt（點）
#include <Fonts/FreeMono12pt7b.h>  // 字型 12pt（點）
#include <Fonts/FreeMono18pt7b.h>  // 字型 18pt（點）
#include <Fonts/FreeMono24pt7b.h>  // 字型 24pt（點）
#include <SPI.h>                   // SPI 通訊函式庫（用於 TFT LCD）

// ===== AVR 平台特定設定 =====
#ifdef __AVR__
#include <avr/power.h>  // AVR 電源管理函式庫（ATtiny85 需要）
#endif

// ===== ATtiny85 時鐘設定 =====
// 如果使用 ATtiny85 且時鐘頻率為 16MHz，設定時鐘預分頻器為 1
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

// ===== 按鍵讀取巨集定義 =====
// 這些巨集用於直接讀取 AVR 微控制器的腳位狀態（速度較快）
#define keyB(byte) (PINB & (1 << (byte)))  // 讀取 PORT B 的指定位元（D8-D13）
#define keyC(byte) (PINC & (1 << (byte)))  // 讀取 PORT C 的指定位元（A0-A5）
#define keyD(byte) (PIND & (1 << (byte)))  // 讀取 PORT D 的指定位元（D0-D7）

// ===== Timer1 計數器變數 =====
// 全域變數，用於儲存 Timer1 的初始計數值
uint16_t timer1_counter;

/*
 * ============================================================================
 * 函式名稱：timer_ini
 * 功能說明：初始化 Timer1 計時器（用於產生週期性中斷）
 * 
 * 參數：
 *   tm - Timer1 的初始計數值（uint16_t, 0-65535）
 * 
 * 計算公式：
 *   中斷頻率 = 16MHz / (預分頻器 × (65536 - tm))
 *   預分頻器 = 256
 *   
 * 範例：
 *   1Hz (每秒一次): tm = 65536 - 62500 = 3036
 *   2Hz (每 0.5 秒): tm = 65536 - 31250 = 34286
 * 
 * 使用方式：
 *   timer_ini(3036);  // 設定為 1Hz
 *   需要配合 ISR(TIMER1_OVF_vect) 中斷服務程式使用
 * ============================================================================
 */
void timer_ini(uint16_t tm){
  noInterrupts();           // 關閉所有中斷（確保設定過程不被打斷）
  
  TCCR1A = 0;               // 清除 Timer1 控制暫存器 A（設為正常模式）
  TCCR1B = 0;               // 清除 Timer1 控制暫存器 B
  
  timer1_counter = tm;      // 儲存計數器初始值（用於中斷後重新載入）
  TCNT1 = timer1_counter;   // 設定 Timer1 計數器初始值
  
  TCCR1B |= (1 << CS12);    // 設定預分頻器為 256 (CS12=1, CS11=0, CS10=0)
                            // 16MHz / 256 = 62.5kHz 計數頻率
  
  TIMSK1 |= (1 << TOIE1);   // 啟用 Timer1 溢位中斷
                            // 當計數器從 65535 溢位到 0 時觸發中斷
  
  interrupts();             // 重新開啟所有中斷
}

/*
 * ============================================================================
 * 函式名稱：convert24to16
 * 功能說明：將 24 位元 RGB 顏色轉換為 16 位元 RGB565 格式
 * 
 * 參數：
 *   rgb - 24 位元顏色值（uint32_t）
 *         格式：0xRRGGBB
 *         例如：0xFF0000 (紅色), 0x00FF00 (綠色), 0x0000FF (藍色)
 * 
 * 返回值：
 *   uint16_t - 16 位元 RGB565 格式顏色
 *              格式：RRRRRGGGGGGBBBBB (5-6-5 位元)
 * 
 * 色彩位元分配：
 *   24 位元 RGB888：R(8位元) G(8位元) B(8位元) = 16,777,216 色
 *   16 位元 RGB565：R(5位元) G(6位元) B(5位元) = 65,536 色
 * 
 * 轉換說明：
 *   - 紅色：8 位元 (0-255) → 5 位元 (0-31)
 *   - 綠色：8 位元 (0-255) → 6 位元 (0-63)
 *   - 藍色：8 位元 (0-255) → 5 位元 (0-31)
 * 
 * 使用範例：
 *   uint32_t color24 = 0xFF5500;  // 橙色 (24 位元)
 *   uint16_t color16 = convert24to16(color24);  // 轉換為 16 位元
 * 
 * 應用場景：
 *   某些 TFT 顯示器（如 ST7735）使用 RGB565 格式，此函式可將
 *   常見的 24 位元顏色碼轉換為顯示器所需的 16 位元格式
 * ============================================================================
 */
uint16_t convert24to16(uint32_t rgb) {
    // 步驟 1：分離 24 位元顏色的 R、G、B 分量
    uint32_t r = (rgb >> 16) & 0xFF;  // 取出紅色分量（右移 16 位元，遮罩 0xFF）
    uint32_t g = (rgb >> 8) & 0xFF;   // 取出綠色分量（右移 8 位元，遮罩 0xFF）
    uint32_t b = rgb & 0xFF;          // 取出藍色分量（直接遮罩 0xFF）
    
    // 步驟 2：將 8 位元色彩值映射到對應的位元數，並移位到正確位置
    r = map(r, 0x00, 0xFF, 0x00, 0x1F) << 11;  // R: 8位元(0-255) → 5位元(0-31)，左移 11 位元
    g = map(g, 0x00, 0xFF, 0x00, 0x3F) << 5;   // G: 8位元(0-255) → 6位元(0-63)，左移 5 位元
    b = map(b, 0x00, 0xFF, 0x00, 0x1F);        // B: 8位元(0-255) → 5位元(0-31)，不需移位
    
    // 步驟 3：組合 RGB565 格式
    // RGB565 格式：[R4 R3 R2 R1 R0 | G5 G4 G3 G2 G1 G0 | B4 B3 B2 B1 B0]
    return r | g | b;  // 使用 OR 運算合併三個顏色分量
}

