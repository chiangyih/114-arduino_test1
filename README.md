# 114工科賽參考 - Arduino 藍牙介面卡韌體(113題目)

## 專案概述

113 學年度全國高級中等學校工業類科學生技藝競賽  
電腦修護職類 第二站 — 個人電腦 USB / 藍牙介面卡製作及控制

### 硬體配置
- **MCU**: ATmega328P (Arduino UNO)
- **通訊**: HC-05 Bluetooth SPP (9600 bps)
- **顯示**: ST7735 1.8" TFT LCD (128×160, Software SPI)
- **RGB**: WS2812 × 8 LEDs (D5)
- **崗位**: 01 (藍牙名稱: ODD-01-0001)

### 主要功能
- **F1**: CPU 運行指示燈閃爍 (D13, 500ms)
- **F2**: TFT 顯示初始化畫面 (TCIVS/C201, 2秒)
- **F3**: RGB Offline 選單 (Red/Green/Blue/Gradient)
- **F4**: CountDown 倒數計時 (10秒 → 0秒)
- **F5**: 藍牙模組命名邏輯 (ODD/EVEN-XX-BBBB)
- **F6**: 藍牙通訊控制 (PING/CONNECT/DISCONNECT)
- **F7**: Connect to BLE 模式 (CPU Loading 顏色顯示)
- **F8**: EEPROM 寫入/讀取 (0-255)

---

## 快速開始

### 1. 環境需求
- PlatformIO (VSCode 擴充套件)
- Arduino UNO 開發板
- HC-05 藍牙模組

### 2. 編譯與上傳
```bash
# 編譯程式碼
pio run

# 上傳到 Arduino
pio run --target upload

# 開啟序列埠監視器 (9600 baud)
pio device monitor
```

### 3. 測試腳本
```bash
# WS2812 顏色測試
python test_ws2812_colors.py

# 藍牙逾時測試
python test_bluetooth_timeout.py

# BLE 連線測試
python test_ble_connection.py
```

---

## 文件索引

### 核心文件
- **[FirmwareSpec.md](FirmwareSpec.md)** - 完整韌體需求規格
- **[Arduino_WS2812_Integration_Guide.md](Arduino_WS2812_Integration_Guide.md)** - WS2812 整合指南

### 快速參考
- **[腳位配置參考卡.md](腳位配置參考卡.md)** - 硬體接線速查表
- **[WS2812色彩對應快速參考表.md](WS2812色彩對應快速參考表.md)** - CPU Loading 顏色對應
- **[藍牙資料格式快速參考.md](藍牙資料格式快速參考.md)** - 藍牙命令格式
- **[Connect_to_BLE功能快速參考.md](Connect_to_BLE功能快速參考.md)** - BLE 連線功能說明

### 硬體設定
- **[ST7735接線說明.md](ST7735接線說明.md)** - TFT 顯示器接線指南
- **[測試指南.md](測試指南.md)** - 系統測試程序

---

## 藍牙通訊協定

### 支援的命令

| 命令 | 格式 | 功能 | 回應 |
|------|------|------|------|
| CONNECT | `CONNECT\n` | 建立藍牙連線 | `ACK\n` |
| DISCONNECT | `DISCONNECT\n` | 中斷藍牙連線 | `ACK\n` |
| PING | `PING\n` | 心跳確認 | `ACK\n` |
| LOAD | `LOAD <0-100>\n` | 設定 CPU Loading 顏色 | `ACK\n` |
| WRITE | `WRITE <0-255>\n` | 寫入 EEPROM | `ACK\n` / `ERR\n` |

### CPU Loading 顏色對應
- **0-50%**: 綠色 (正常負載)
- **51-84%**: 黃色 (中度負載)
- **85-100%**: 紅色 (高負載)

### 連線逾時機制
- 逾時時間: 5 秒
- 超過 5 秒未收到資料自動顯示 "Disconnect"
- TFT 自動更新連線狀態

---

## 開發歷程

### 已完成項目
✅ 基本功能實作 (F1-F8)  
✅ WS2812 顏色閾值修正 (符合整合指南)  
✅ 藍牙逾時偵測機制  
✅ TFT 顯示自動更新  
✅ EEPROM 讀寫功能  
✅ 完整測試腳本  
✅ Serial Monitor 除錯輸出 (9600 baud)

### 測試狀態
⚠️ 實機硬體測試待完成  
⚠️ PC 端監控程式整合測試待完成

---

## 故障排除

### TFT 顯示異常
1. 檢查接線 (參考 ST7735接線說明.md)
2. 確認使用 Software SPI 模式 (MOSI=A4, SCK=A5)
3. 嘗試不同初始化參數 (INITR_BLACKTAB/GREENTAB/REDTAB)

### 藍牙連線問題
1. 確認 HC-05 鮑率為 9600 bps
2. 檢查 Serial Monitor 是否顯示 "BLE RX:" 訊息
3. 確認 platformio.ini 設定 `monitor_speed = 9600`

### WS2812 不亮
1. 檢查資料線接在 D5
2. 確認電源供應足夠 (5V, ≥2A)
3. 檢查 CPU Loading 數值範圍 (0-100)

---

## 授權與貢獻

本專案為教育用途開發  
© 2024 113學年度工科賽參賽作品

**最後更新**: 2025年11月19日
