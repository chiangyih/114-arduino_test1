# Connect to BLE 功能快速參考

## 功能概述
選單 "1.Connect to BLE" 用於接收 PC 端透過 HC-05 藍牙模組發送的資料。

---

## 硬體連接

### HC-05 接線
```
HC-05 TX  →  Arduino RX (Pin 0)
HC-05 RX  →  Arduino TX (Pin 1)
HC-05 VCC →  Arduino 5V
HC-05 GND →  Arduino GND
```

### 藍牙配對
1. PC 端搜尋藍牙裝置
2. 裝置名稱：HC-05（或自訂）
3. 配對密碼：`1234` 或 `0000`
4. 記下配對後的 COM port

---

## 支援的命令

| 命令 | 格式 | 功能 | 回應 | 範例 |
|------|------|------|------|------|
| CONNECT | `CONNECT` | 建立連線 | `ACK` | `CONNECT` |
| DISCONNECT | `DISCONNECT` | 中斷連線 | `ACK` | `DISCONNECT` |
| PING | `PING` | 心跳確認 | `ACK` | `PING` |
| LOAD | `LOAD <VAL>` | 更新 WS2812 顏色 | `ACK` | `LOAD 65` |
| WRITE | `WRITE <DEC>` | 寫入 EEPROM | `ACK`/`ERR` | `WRITE 123` |

---

## WS2812 色彩對應

| CPU % | 顏色 | RGB 值 | 說明 |
|-------|------|--------|------|
| 0-50 | 🟢 綠色 | (0,255,0) | 正常負載 |
| 51-84 | 🟡 黃色 | (255,255,0) | 中度負載 |
| 85-100 | 🔴 紅色 | (255,0,0) | 高負載 |

---

## TFT 顯示狀態

| 狀態 | 顯示文字 | 顏色 | 說明 |
|------|---------|------|------|
| 已連線 | Connected | 綠色 | 正常接收資料 |
| 已中斷 | Disconnect | 紅色 | 無連線或已中斷 |

---

## 快速測試命令

### 測試 1：建立連線
```
CONNECT
→ TFT 顯示 "Connected"（綠色）
→ 回傳 "ACK"
```

### 測試 2：綠色 LED
```
LOAD 30
→ WS2812 顯示綠色（8 顆全亮）
→ 回傳 "ACK"
```

### 測試 3：黃色 LED
```
LOAD 65
→ WS2812 顯示黃色（8 顆全亮）
→ 回傳 "ACK"
```

### 測試 4：紅色 LED
```
LOAD 90
→ WS2812 顯示紅色（8 顆全亮）
→ 回傳 "ACK"
```

### 測試 5：寫入 EEPROM
```
WRITE 123
→ EEPROM 儲存 123
→ 回傳 "ACK"
```

### 測試 6：中斷連線
```
DISCONNECT
→ TFT 顯示 "Disconnect"（紅色）
→ WS2812 全部關閉
→ 回傳 "ACK"
```

---

## 自動化測試

### 使用 Python 腳本
```bash
python test_ble_connection.py
```

### 測試項目
1. ✅ CONNECT 命令
2. ✅ PING 命令
3. ✅ LOAD 30（綠色）
4. ✅ LOAD 65（黃色）
5. ✅ LOAD 90（紅色）
6. ✅ 邊界值測試（0, 50, 51, 84, 85, 100）
7. ✅ WRITE 123
8. ✅ 持續心跳（5 次 PING）
9. ✅ DISCONNECT 命令
10. ✅ 自動重新連線

---

## 特殊功能

### 自動連線偵測
無需手動發送 CONNECT，只要收到任何資料就自動連線。

**範例**：
```
LOAD 50  （直接發送，無需先 CONNECT）
→ 自動設定為連線狀態
→ TFT 顯示 "Connected"
→ WS2812 顯示綠色
```

### 逾時自動中斷
5 秒未收到任何資料，自動判定為中斷連線。

**測試方法**：
1. 建立連線
2. 停止發送資料
3. 等待 6 秒
4. TFT 自動顯示 "Disconnect"

---

## 常見問題

### Q1: TFT 沒顯示 "Connected"？
**A**: 確認已進入 "Connect to BLE" 選單（選單 1）

### Q2: 沒收到 ACK 回應？
**A**: 檢查 HC-05 接線，TX/RX 是否交叉連接

### Q3: WS2812 顏色錯誤？
**A**: 確認 LOAD 數值，參考色彩對應表

### Q4: 自動中斷連線？
**A**: PC 端需定期發送 PING（建議每 2 秒）

### Q5: WRITE 收到 ERR？
**A**: 確認數值在 0-255 範圍內

---

## 序列埠設定

| 參數 | 值 |
|------|-----|
| Baud Rate | 9600 bps |
| Data Bits | 8 |
| Parity | None |
| Stop Bits | 1 |
| Flow Control | None |

---

## Python 範例程式碼

### 基本發送
```python
import serial
import time

ser = serial.Serial('COM5', 9600)
time.sleep(2)

# 發送命令
ser.write(b'LOAD 50\n')
time.sleep(0.2)

# 讀取回應
response = ser.readline().decode().strip()
print(response)  # 應顯示 "ACK"

ser.close()
```

### 持續心跳
```python
import threading
import time

def heartbeat(ser):
    while True:
        ser.write(b'PING\n')
        time.sleep(2)

thread = threading.Thread(target=heartbeat, args=(ser,), daemon=True)
thread.start()
```

---

## 檔案清單

- `src/main.cpp` - Arduino 韌體
- `test_ble_connection.py` - 自動化測試腳本
- `Connect_to_BLE功能檢查報告.md` - 詳細檢查報告
- `Connect_to_BLE功能快速參考.md` - 本檔案

---

**最後更新**：2025-01-11  
**韌體版本**：v1.0  
**測試狀態**：✅ 程式碼檢查通過
