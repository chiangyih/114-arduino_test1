# TFT 腳位修正摘要

## ⚠️ 重要修正

**實際硬體配置與規格文件不同！**

---

## 🔄 修正內容

### 最新修正：使用軟體 SPI 模式

**重要發現：TFT 使用軟體 SPI，MOSI 和 SCK 腳位可自訂！**

```cpp
// 軟體 SPI 腳位定義
#define TFT_CS   10       // Chip Select
#define TFT_DC   8        // Data/Command
#define TFT_RST  9        // Reset
#define TFT_MOSI A4       // SDA (MOSI) - 軟體 SPI ✅
#define TFT_SCLK A5       // SCL (SCK) - 軟體 SPI ✅
#define TFT_BL   6        // 背光控制
```

---

## 📌 完整腳位表（軟體 SPI 模式）

| 功能 | 實際接腳 | 說明 |
|------|---------|------|
| **SDA (MOSI)** | **A4** | **SPI 資料線** ✅ |
| **SCL (SCK)** | **A5** | **SPI 時鐘線** ✅ |
| CS | D10 | 晶片選擇 |
| DC | D8 | 資料/命令 |
| RST | D9 | 重置 |
| VCC | 5V | 電源 |
| GND | GND | 接地 |
| BL | D6 | 背光 |

---

## 🎯 記憶口訣

```
軟體 SPI 模式：
SDA=A4, SCL=A5 (資料和時鐘)
CS=D10, DC=D8, RST=D9 (控制腳位)
```

---

## 📝 已更新的文件

✅ `src/main.cpp` - 程式碼腳位定義
✅ `腳位配置參考卡.md` - 接線圖和定義
✅ `測試指南.md` - 硬體連接檢查表
✅ `TFT白屏問題排除.md` - 接線檢查
✅ `ST7735接線說明.md` - 完整接線文件

---

## 🔌 接線圖（軟體 SPI 模式）

```
TFT 模組 → Arduino UNO
========   ============
SDA     →  A4 (MOSI) ✅ 軟體 SPI
SCL     →  A5 (SCK)  ✅ 軟體 SPI
CS      →  D10
DC      →  D8
RST     →  D9
VCC     →  5V
GND     →  GND
BL      →  D6
```

---

## ⚡ 快速驗證

編譯並上傳程式後：
1. 螢幕應該顯示紅藍測試條
2. 然後顯示「TCIVS」和「C201」
3. 2 秒後進入主選單

如果仍然白屏，請檢查：
- SDA 是否確實連接到 A4
- SCL 是否確實連接到 A5
- CS/DC/RST 接線是否正確
- 初始化類型（試試 INITR_GREENTAB）

---

**修正日期**：2025/11/10  
**版本**：v2.0（軟體 SPI 模式）
**狀態**：已更新為軟體 SPI 配置
