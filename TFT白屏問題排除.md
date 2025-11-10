# TFT 白屏問題排除指南

## 問題現象
TFT 顯示器只顯示白色畫面，沒有任何圖形或文字。

---

## 常見原因與解決方法

### 1. 初始化類型不正確
**原因**：ST7735 有不同版本（黑色標籤、綠色標籤、紅色標籤），初始化方法不同。

**解決方法**：
在 `main.cpp` 的 setup() 函式中，嘗試不同的初始化方法：

```cpp
// 方法 1：黑色標籤（預設）
tft.initR(INITR_BLACKTAB);

// 方法 2：綠色標籤（如果方法 1 不行，試試這個）
tft.initR(INITR_GREENTAB);

// 方法 3：紅色標籤
tft.initR(INITR_REDTAB);

// 方法 4：1.44" 綠色標籤
tft.initR(INITR_144GREENTAB);
```

---

### 2. 接線錯誤
**檢查接線**（軟體 SPI 配置）：

| 順序 | TFT 腳位 | Arduino 腳位 | 功能 |
|------|----------|--------------|------|
| 1 | SDA/MOSI | **A4** | SPI 資料線（軟體 SPI）✅ |
| 2 | SCL/SCK | **A5** | SPI 時鐘線（軟體 SPI）✅ |
| 3 | CS | D10 | 晶片選擇 |
| 4 | DC/A0 | D8 | 資料/命令選擇 |
| 5 | RST | D9 | 重置 |
| 6 | VCC | 5V | 電源 |
| 7 | GND | GND | 接地 |
| 8 | LED/BL | D6 | 背光控制 |

> 📝 **軟體 SPI 配置**：SDA=A4, SCL=A5, CS=D10, DC=D8, RST=D9

**常見錯誤**：
- SDA 接到 D11 而非 A4（必須是 A4）
- SCL 接到 D13 而非 A5（必須是 A5）
- CS 和 DC 腳位接反
- 電源不足（建議使用外部電源）

---

### 3. 螢幕方向設定
**原因**：某些螢幕需要特定的旋轉設定才能正確顯示。

**解決方法**：
嘗試不同的旋轉值（0-3）：

```cpp
tft.setRotation(0);  // 直向
tft.setRotation(1);  // 橫向（預設）
tft.setRotation(2);  // 倒轉直向
tft.setRotation(3);  // 倒轉橫向
```

---

### 4. SPI 通訊衝突
**原因**：多個 SPI 裝置可能造成衝突。

**解決方法**：
- 確保每個 SPI 裝置有獨立的 CS 腳位
- 檢查 WS2812 是否影響（暫時註解掉測試）

---

### 5. 電源問題
**原因**：TFT 和其他模組同時使用可能導致電源不足。

**解決方法**：
- 使用外部 5V 電源供應器（至少 1A）
- 不要只靠 USB 供電
- 檢查電源線和接地線是否良好

---

## 🔧 快速測試程式

### 測試程式 1：基本顏色測試

在 `setup()` 函式中加入以下測試程式碼：

```cpp
void setup() {
  Serial.begin(9600);
  Serial.println("TFT Test Starting...");
  
  // 初始化 TFT
  tft.initR(INITR_BLACKTAB);  // 嘗試不同版本
  delay(100);
  
  Serial.println("TFT Initialized");
  
  // 測試 1：純色填充
  tft.fillScreen(ST77XX_RED);
  delay(1000);
  Serial.println("RED");
  
  tft.fillScreen(ST77XX_GREEN);
  delay(1000);
  Serial.println("GREEN");
  
  tft.fillScreen(ST77XX_BLUE);
  delay(1000);
  Serial.println("BLUE");
  
  tft.fillScreen(ST77XX_BLACK);
  delay(1000);
  Serial.println("BLACK");
  
  // 測試 2：繪製圖形
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(10, 10, 100, 60, ST77XX_YELLOW);  // 矩形
  tft.fillCircle(64, 64, 30, ST77XX_MAGENTA);    // 圓形
  delay(2000);
  
  // 測試 3：顯示文字
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Hello!");
  tft.println("TFT Works");
  
  Serial.println("Test Complete");
}

void loop() {
  // 空的
}
```

### 測試程式 2：序列埠除錯

```cpp
void testTFT() {
  Serial.println("=== TFT Diagnostic ===");
  
  // 測試 1：檢查 CS 腳位
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, LOW);
  Serial.print("CS Pin: ");
  Serial.println(digitalRead(TFT_CS) == LOW ? "LOW (OK)" : "HIGH (Error)");
  
  // 測試 2：檢查 RST 腳位
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);
  Serial.print("RST Pin: ");
  Serial.println(digitalRead(TFT_RST) == HIGH ? "HIGH (OK)" : "LOW (Error)");
  
  // 測試 3：初始化並繪製
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_GREEN);
  Serial.println("Screen filled with GREEN");
  
  delay(2000);
  
  tft.fillScreen(ST77XX_BLUE);
  Serial.println("Screen filled with BLUE");
}
```

---

## 📋 逐步除錯流程

### 步驟 1：檢查硬體
- [ ] 確認所有接線正確
- [ ] 確認電源供應充足
- [ ] 確認 TFT 模組無損壞

### 步驟 2：測試基本通訊
- [ ] 上傳測試程式 1（純色測試）
- [ ] 觀察螢幕是否有顏色變化
- [ ] 檢查序列埠輸出

### 步驟 3：嘗試不同初始化
- [ ] 嘗試 `INITR_BLACKTAB`
- [ ] 嘗試 `INITR_GREENTAB`
- [ ] 嘗試 `INITR_REDTAB`
- [ ] 嘗試 `INITR_144GREENTAB`

### 步驟 4：調整旋轉設定
- [ ] 嘗試 `setRotation(0)`
- [ ] 嘗試 `setRotation(1)`
- [ ] 嘗試 `setRotation(2)`
- [ ] 嘗試 `setRotation(3)`

### 步驟 5：檢查函式庫版本
- [ ] 確認使用 Adafruit ST7735 Library v1.11.0
- [ ] 確認使用 Adafruit GFX Library v1.12.3

---

## 🎯 最可能的解決方案

根據經驗，ST7735 白屏問題最常見的原因是：

### 解決方案 1：更改初始化類型（90% 機率）

```cpp
// 從這個：
tft.initR(INITR_BLACKTAB);

// 改成這個：
tft.initR(INITR_GREENTAB);
```

### 解決方案 2：添加延遲（5% 機率）

```cpp
tft.initR(INITR_BLACKTAB);
delay(500);  // 增加延遲時間
tft.setRotation(1);
```

### 解決方案 3：調整偏移（5% 機率）

某些 ST7735 模組需要設定偏移：

```cpp
// 在初始化後添加
tft.setColStart(0);
tft.setRowStart(0);
```

---

## 📞 進階除錯資訊

如果以上方法都無效，請提供：

1. TFT 模組型號和標籤顏色（黑/綠/紅）
2. 螢幕尺寸（1.44" / 1.8"）
3. 序列埠輸出訊息
4. 是否有任何閃爍或顏色變化
5. 使用的 Arduino 板子型號

---

## ✅ 確認修復

當 TFT 正常工作時，你應該看到：

1. **開機畫面**：
   - 頂部紅色條（測試用）
   - 底部藍色條（測試用）
   - 黑色背景
   - 白色文字「TCIVS」和「C201」

2. **主選單**：
   - 藍色標題「MENU」
   - 四個白色選項
   - 藍色反白選擇

---

**最後更新**：2025/11/10
**版本**：v1.1
