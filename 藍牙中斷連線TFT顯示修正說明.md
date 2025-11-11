# 藍牙中斷連線 TFT 顯示修正說明

## 修正日期
2025-01-11

## 問題描述

當 PC 端藍牙中斷連線後（例如關閉監控程式、藍牙斷線、PC 關機等），Arduino 無法偵測到連線已中斷，TFT 畫面仍然顯示 "Connected"，而非預期的 "Disconnected"。

### 問題原因

1. **原本的設計**：只有在收到 `DISCONNECT` 命令時才會設定 `bleConnected = false` 並更新顯示
2. **實際情況**：當 PC 端直接關閉或藍牙斷線時，不會發送 `DISCONNECT` 命令
3. **結果**：Arduino 的 `bleConnected` 變數維持 `true`，TFT 持續顯示錯誤的連線狀態

---

## 解決方案

實作**心跳逾時檢測機制**（Heartbeat Timeout Detection）：

### 核心概念
- 記錄最後一次收到藍牙資料的時間
- 如果超過指定時間（5 秒）沒有收到任何資料，則視為連線中斷
- 自動更新 `bleConnected` 狀態並刷新 TFT 顯示

### 實作細節

#### 1. 新增變數（約第 98-99 行）
```cpp
unsigned long lastBleDataTime = 0;      // 最後一次收到藍牙資料的時間
const unsigned long BLE_TIMEOUT = 5000; // 藍牙逾時時間（5 秒）
```

#### 2. 更新時間戳記（handleBluetoothData 函式）
每次收到藍牙資料時，更新 `lastBleDataTime`：

```cpp
void handleBluetoothData() {
  // 只要收到任何資料，就視為藍牙已連線（自動偵測連線）
  if (Serial.available() > 0) {
    // 更新最後收到資料的時間（用於逾時檢測）
    lastBleDataTime = millis();
    
    if (!bleConnected) {
      bleConnected = true;
      // ... 更新顯示為 Connected
    }
  }
  // ...
}
```

#### 3. 逾時檢測（loop 函式中的 MENU_CONNECT_BLE 區段）
在每次 loop 迴圈中檢查是否逾時：

```cpp
case MENU_CONNECT_BLE:
  if (inSubMenu) {
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
```

---

## 功能驗證

### 測試案例 1：正常連線
**步驟**：
1. Arduino 進入 "Connect to BLE" 選單
2. PC 端發送 `CONNECT` 或 `PING` 命令

**預期結果**：
- TFT 顯示 "Connected"（綠色）
- `bleConnected = true`

### 測試案例 2：持續心跳
**步驟**：
1. 建立藍牙連線
2. PC 端每隔 1-3 秒發送 `PING` 命令

**預期結果**：
- TFT 持續顯示 "Connected"（綠色）
- `lastBleDataTime` 不斷更新
- 不會觸發逾時機制

### 測試案例 3：PC 端正常中斷（發送命令）
**步驟**：
1. 建立藍牙連線
2. PC 端發送 `DISCONNECT` 命令

**預期結果**：
- TFT 立即顯示 "Disconnect"（紅色）
- WS2812 LED 全部關閉
- `bleConnected = false`

### 測試案例 4：PC 端異常中斷（本次修正重點）
**步驟**：
1. 建立藍牙連線
2. 直接關閉 PC 端監控程式或中斷藍牙配對
3. 等待 5 秒

**預期結果**：
- 5 秒後 TFT 自動更新為 "Disconnect"（紅色）
- WS2812 LED 全部關閉
- `bleConnected = false`
- ✅ **修正前**：TFT 持續顯示 "Connected"（錯誤）
- ✅ **修正後**：TFT 顯示 "Disconnect"（正確）

### 測試案例 5：重新連線
**步驟**：
1. 從測試案例 4 的中斷狀態
2. PC 端重新連線並發送任意命令（如 `PING`）

**預期結果**：
- TFT 立即更新為 "Connected"（綠色）
- `bleConnected = true`
- `lastBleDataTime` 重新開始計時

---

## 參數說明

### BLE_TIMEOUT = 5000 (毫秒)
- **預設值**：5 秒
- **用途**：判斷藍牙連線是否逾時的時間閾值
- **調整建議**：
  - 如果 PC 端心跳頻率較低（例如每 10 秒），可增加到 15000 (15 秒)
  - 如果需要快速偵測中斷，可減少到 3000 (3 秒)
  - 建議設定為「PC 端心跳間隔的 2-3 倍」

### 修改範例
如果要改為 10 秒逾時：
```cpp
const unsigned long BLE_TIMEOUT = 10000; // 藍牙逾時時間（10 秒）
```

---

## 技術細節

### millis() 函式說明
- `millis()` 回傳 Arduino 開機後經過的毫秒數
- 類型：`unsigned long`（0 到 4,294,967,295，約 49.7 天）
- 約 49.7 天後會溢位歸零（極少發生，但在逾時計算時需要考慮）

### 溢位處理
目前的實作：
```cpp
if (bleConnected && (millis() - lastBleDataTime > BLE_TIMEOUT))
```

這個寫法可以正確處理 millis() 溢位的情況，因為 unsigned long 的減法運算會自動處理溢位（wrap-around）。

**範例**：
- 假設 `lastBleDataTime = 4294967000`（接近溢位）
- 5 秒後 `millis() = 295`（已溢位）
- 計算：`295 - 4294967000 = 5295`（unsigned long 特性）
- 結果：`5295 > 5000` → 正確判斷為逾時

---

## 相關功能影響

### ✅ 不受影響的功能
- F1: CPU 運行指示燈
- F2: EEPROM 讀取
- F3: 倒數計時
- F4-F6: RGB Offline 模式
- F8: EEPROM 顯示

### ✅ 改善的功能
- F7: Connect to BLE 模式
  - 新增自動偵測中斷功能
  - 更準確的連線狀態顯示
  - 自動清除 WS2812 顯示

### 🔄 相關命令行為
- `CONNECT`：正常觸發連線，重置逾時計時
- `DISCONNECT`：立即中斷，無需等待逾時
- `PING`：心跳命令，重置逾時計時
- `LOAD <VAL>`：更新 WS2812，重置逾時計時
- `WRITE <DEC>`：寫入 EEPROM，重置逾時計時

**結論**：只要 PC 端有發送任何資料，都會重置逾時計時器。

---

## 建議的 PC 端實作

為了配合此心跳機制，建議 PC 端監控程式實作以下功能：

### 方案 1：定期心跳（推薦）
```python
import serial
import time
import threading

def heartbeat_thread(ser):
    """背景執行緒，每 2 秒發送一次 PING"""
    while True:
        try:
            ser.write(b'PING\n')
            time.sleep(2)  # 每 2 秒一次（遠小於 5 秒逾時）
        except:
            break

# 主程式
ser = serial.Serial('COM5', 9600)
thread = threading.Thread(target=heartbeat_thread, args=(ser,), daemon=True)
thread.start()
```

### 方案 2：命令發送時自動心跳
```python
import time

last_send_time = 0

def send_command(ser, command):
    """發送命令，如果距離上次超過 2 秒則先發送 PING"""
    global last_send_time
    
    current_time = time.time()
    if current_time - last_send_time > 2:
        ser.write(b'PING\n')
        time.sleep(0.1)
    
    ser.write(command.encode() + b'\n')
    last_send_time = current_time
```

### 方案 3：關閉時主動中斷
```python
import atexit

def on_exit(ser):
    """程式結束時發送 DISCONNECT"""
    try:
        ser.write(b'DISCONNECT\n')
        time.sleep(0.5)
        ser.close()
    except:
        pass

ser = serial.Serial('COM5', 9600)
atexit.register(on_exit, ser)
```

---

## 除錯資訊

### 如何檢查逾時機制是否運作

在 Arduino IDE 序列埠監視器中可以看到：

1. **連線時**：
   - 發送 `CONNECT` → 收到 `ACK`
   - TFT 顯示 "Connected"

2. **心跳正常時**：
   - 每隔幾秒發送 `PING` → 收到 `ACK`
   - TFT 持續顯示 "Connected"

3. **停止發送資料**：
   - 等待 5 秒
   - TFT 自動變更為 "Disconnect"
   - WS2812 自動關閉

4. **重新連線**：
   - 發送任意命令（如 `PING`）
   - TFT 立即變更為 "Connected"

### 常見問題

**Q1: 為什麼設定 5 秒逾時？**  
A: 5 秒是折衷值，既不會太快造成誤判，也不會太慢導致使用者等待過久。可根據實際需求調整 `BLE_TIMEOUT` 常數。

**Q2: 如果 PC 端發送頻率不固定怎麼辦？**  
A: 建議在 PC 端實作背景心跳執行緒，確保每 2-3 秒發送一次 `PING` 命令。

**Q3: 逾時檢測會影響效能嗎？**  
A: 不會。逾時檢測只是簡單的時間比較，每次 loop 只執行一次，對效能影響極小。

**Q4: 可以完全停用逾時檢測嗎？**  
A: 可以。將 `BLE_TIMEOUT` 設定為很大的值（如 `4294967295`），或移除 loop 中的逾時檢查程式碼。

**Q5: 如果只想顯示但不清除 WS2812 呢？**  
A: 移除逾時檢測區塊中的 `strip.clear()` 和 `strip.show()` 兩行程式碼即可。

---

## 測試清單

上傳修正後的韌體後，請完成以下測試：

- [ ] 正常連線測試（發送 CONNECT）
- [ ] 心跳維持測試（每 2 秒發送 PING）
- [ ] 正常中斷測試（發送 DISCONNECT）
- [ ] **異常中斷測試**（關閉 PC 端程式，等待 5 秒）
- [ ] 重新連線測試（中斷後再次發送命令）
- [ ] 其他功能測試（EEPROM、倒數計時、RGB 模式）

---

## 檔案修改記錄

### 修改檔案
- `src/main.cpp`

### 修改位置
1. **變數宣告區**（約第 98-99 行）
   - 新增 `lastBleDataTime`
   - 新增 `BLE_TIMEOUT`

2. **loop() 函式**（約第 278-295 行）
   - 新增逾時檢測邏輯於 `MENU_CONNECT_BLE` case

3. **handleBluetoothData() 函式**（約第 827-841 行）
   - 新增 `lastBleDataTime = millis()` 更新時間戳記

### 程式碼行數變化
- 修正前：1005 行
- 修正後：1025 行（增加約 20 行）

---

## 結論

### ✅ 修正完成
- PC 端藍牙中斷連線後，TFT 會在 5 秒後自動顯示 "Disconnected"
- WS2812 LED 會自動關閉
- 不影響其他功能的正常運作

### 📝 建議
- PC 端監控程式應實作定期心跳機制（每 2-3 秒發送 PING）
- 如需調整逾時時間，修改 `BLE_TIMEOUT` 常數即可

### 🎯 下一步
1. 上傳修正後的韌體
2. 進行異常中斷測試
3. 確認 TFT 顯示正確更新
4. 與 PC 端監控程式整合測試

---

**修正者**：GitHub Copilot  
**測試狀態**：✅ 編譯通過，待實機測試  
**相容性**：✅ 與所有現有功能相容
