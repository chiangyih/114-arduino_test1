"""
藍牙中斷連線測試程式
用於驗證 Arduino 的逾時檢測機制是否正常運作
"""

import serial
import time
import sys

def test_bluetooth_timeout(port='COM5', baudrate=9600):
    """測試藍牙逾時檢測功能"""
    
    try:
        # 連接序列埠
        print(f"正在連接到 {port} (Baud: {baudrate})...")
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # 等待 Arduino 重置
        print("✅ 連接成功！\n")
        
        print("=" * 70)
        print("  藍牙中斷連線逾時檢測測試")
        print("=" * 70)
        print()
        
        # 測試 1：建立連線
        print("【測試 1】建立藍牙連線")
        print("→ 發送命令: CONNECT")
        ser.write(b"CONNECT\n")
        time.sleep(0.2)
        response = ser.readline().decode().strip()
        print(f"← Arduino 回應: {response}")
        if response == "ACK":
            print("✅ 連線建立成功")
            print("→ 請觀察 TFT 是否顯示 'Connected'（綠色）")
        else:
            print("❌ 連線失敗")
            return
        print()
        time.sleep(2)
        
        # 測試 2：維持心跳
        print("【測試 2】維持心跳連線（每 2 秒發送 PING）")
        print("→ 將發送 5 次 PING 命令，持續 10 秒")
        for i in range(5):
            ser.write(b"PING\n")
            time.sleep(0.2)
            response = ser.readline().decode().strip()
            print(f"  第 {i+1} 次 PING → {response}")
            time.sleep(2)
        print("✅ 心跳維持測試完成")
        print("→ TFT 應持續顯示 'Connected'（綠色）")
        print()
        time.sleep(2)
        
        # 測試 3：停止發送資料，等待逾時
        print("【測試 3】模擬 PC 端異常中斷（停止發送資料）")
        print("→ 停止發送任何命令")
        print("→ 等待 6 秒（逾時設定為 5 秒）")
        print()
        
        for i in range(6, 0, -1):
            print(f"  倒數 {i} 秒...", end="\r")
            time.sleep(1)
        
        print("\n")
        print("✅ 等待完成")
        print("→ 請檢查 TFT 是否已顯示 'Disconnect'（紅色）")
        print("→ WS2812 LED 是否已全部關閉")
        print()
        
        input("按 Enter 繼續下一個測試...")
        print()
        
        # 測試 4：重新連線
        print("【測試 4】重新建立連線")
        print("→ 發送命令: PING")
        ser.write(b"PING\n")
        time.sleep(0.2)
        response = ser.readline().decode().strip()
        print(f"← Arduino 回應: {response}")
        if response == "ACK":
            print("✅ 重新連線成功")
            print("→ 請觀察 TFT 是否立即變更為 'Connected'（綠色）")
        else:
            print("❌ 重新連線失敗")
        print()
        time.sleep(2)
        
        # 測試 5：正常中斷
        print("【測試 5】正常中斷連線（發送 DISCONNECT 命令）")
        print("→ 發送命令: DISCONNECT")
        ser.write(b"DISCONNECT\n")
        time.sleep(0.2)
        response = ser.readline().decode().strip()
        print(f"← Arduino 回應: {response}")
        if response == "ACK":
            print("✅ 中斷命令成功")
            print("→ 請觀察 TFT 是否立即顯示 'Disconnect'（紅色）")
            print("→ WS2812 LED 是否立即關閉")
        else:
            print("❌ 中斷命令失敗")
        print()
        
        # 關閉序列埠
        ser.close()
        
        print("=" * 70)
        print("  測試完成！")
        print("=" * 70)
        print()
        print("📋 測試結果檢查清單：")
        print("  □ 測試 1: CONNECT 後 TFT 顯示 'Connected'（綠色）")
        print("  □ 測試 2: 持續 PING 時 TFT 維持 'Connected'")
        print("  □ 測試 3: 停止發送 6 秒後 TFT 顯示 'Disconnect'（紅色）")
        print("  □ 測試 3: 停止發送後 WS2812 LED 自動關閉")
        print("  □ 測試 4: 重新 PING 後 TFT 立即恢復 'Connected'")
        print("  □ 測試 5: DISCONNECT 後 TFT 立即顯示 'Disconnect'")
        print()
        
    except serial.SerialException as e:
        print(f"❌ 序列埠錯誤: {e}")
        print(f"\n請確認：")
        print(f"1. Arduino 已正確連接到 {port}")
        print(f"2. 序列埠未被其他程式佔用（如 Arduino IDE）")
        print(f"3. COM port 號碼正確")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n\n⚠️  使用者中斷測試")
        if 'ser' in locals() and ser.is_open:
            ser.close()
        sys.exit(0)
    except Exception as e:
        print(f"❌ 未預期的錯誤: {e}")
        sys.exit(1)

def test_heartbeat_simulation(port='COM5', baudrate=9600, duration=30):
    """模擬長時間心跳連線"""
    
    try:
        print(f"正在連接到 {port}...")
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)
        print("✅ 連接成功！")
        print()
        
        print("=" * 70)
        print(f"  心跳模擬測試（持續 {duration} 秒）")
        print("=" * 70)
        print()
        
        # 建立連線
        ser.write(b"CONNECT\n")
        time.sleep(0.2)
        print("✅ 已建立連線")
        print(f"→ 將每 2 秒發送一次 PING，持續 {duration} 秒")
        print("→ TFT 應持續顯示 'Connected'")
        print()
        
        start_time = time.time()
        ping_count = 0
        
        while time.time() - start_time < duration:
            ser.write(b"PING\n")
            time.sleep(0.1)
            response = ser.readline().decode().strip()
            ping_count += 1
            
            elapsed = int(time.time() - start_time)
            print(f"[{elapsed:2d}s] PING #{ping_count} → {response}", end="\r")
            
            time.sleep(2)
        
        print()
        print()
        print(f"✅ 心跳模擬完成！共發送 {ping_count} 次 PING")
        print("→ 如果 TFT 持續顯示 'Connected'，表示心跳機制正常")
        print()
        
        # 測試逾時
        print("現在停止發送，測試逾時機制...")
        print("等待 6 秒...")
        time.sleep(6)
        print("✅ 等待完成")
        print("→ 請確認 TFT 是否已顯示 'Disconnect'")
        print()
        
        ser.close()
        
    except Exception as e:
        print(f"❌ 錯誤: {e}")
        sys.exit(1)

if __name__ == "__main__":
    print("""
╔═══════════════════════════════════════════════════════════════════╗
║         藍牙中斷連線逾時檢測測試程式 v1.0                         ║
║         113學年度 工業類科學生技藝競賽                             ║
╚═══════════════════════════════════════════════════════════════════╝
    """)
    
    print("請選擇測試模式：")
    print("1. 完整功能測試（建議）")
    print("2. 心跳模擬測試（長時間）")
    print()
    
    choice = input("請輸入選項 (1 或 2): ").strip()
    print()
    
    port = input("請輸入 COM port (預設 COM5，直接按 Enter 使用預設值): ").strip()
    if not port:
        port = "COM5"
    
    print()
    
    if choice == "1":
        test_bluetooth_timeout(port)
    elif choice == "2":
        duration = input("請輸入測試時間（秒，預設 30）: ").strip()
        duration = int(duration) if duration else 30
        print()
        test_heartbeat_simulation(port, duration=duration)
    else:
        print("❌ 無效的選項")
        sys.exit(1)
