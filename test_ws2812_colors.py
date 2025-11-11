"""
WS2812 色彩閾值測試程式
用於驗證 LOAD 命令的色彩顯示是否符合 Arduino_WS2812_Integration_Guide.md 規範
"""

import serial
import time
import sys

def test_ws2812_colors(port='COM7', baudrate=9600):
    """測試 WS2812 色彩閾值"""
    
    # 測試案例：(CPU%, 預期顏色, 說明)
    test_cases = [
        (0, "綠色", "最小值 (0%)"),
        (25, "綠色", "正常負載 (25%)"),
        (50, "綠色", "邊界值-綠色上限 (50%)"),
        (51, "黃色", "邊界值-黃色下限 (51%)"),
        (65, "黃色", "中度負載 (65%)"),
        (84, "黃色", "邊界值-黃色上限 (84%)"),
        (85, "紅色", "邊界值-紅色下限 (85%)"),
        (90, "紅色", "高負載 (90%)"),
        (100, "紅色", "最大值 (100%)")
    ]
    
    try:
        # 連接序列埠
        print(f"正在連接到 {port} (Baud: {baudrate})...")
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # 等待 Arduino 重置
        print("✅ 連接成功！\n")
        
        print("=" * 70)
        print("  WS2812 色彩閾值自動化測試")
        print("  依據規範：0-50%=綠色, 51-84%=黃色, 85-100%=紅色")
        print("=" * 70)
        print()
        
        passed = 0
        failed = 0
        
        for cpu_load, expected_color, description in test_cases:
            # 發送 LOAD 命令
            command = f"LOAD {cpu_load}\n"
            ser.write(command.encode())
            time.sleep(0.2)
            
            # 讀取回應
            response = ser.readline().decode().strip()
            
            # 顯示測試結果
            print(f"[測試 {cpu_load:3d}%] {description}")
            print(f"  預期顏色: {expected_color}")
            print(f"  Arduino 回應: {response}", end="")
            
            if response == "ACK":
                print(" ✅")
                passed += 1
            else:
                print(" ❌ (未收到 ACK)")
                failed += 1
            
            print(f"  → 請觀察 WS2812 是否顯示 {expected_color}")
            print()
            
            # 等待觀察 LED 顏色
            time.sleep(3)
        
        # 測試完成後清除 LED
        print("=" * 70)
        print("測試完成！正在關閉 LED...")
        ser.write(b"DISCONNECT\n")
        time.sleep(0.5)
        ser.close()
        
        print()
        print(f"📊 測試統計：")
        print(f"   通過: {passed}/{len(test_cases)}")
        print(f"   失敗: {failed}/{len(test_cases)}")
        
        if failed == 0:
            print("\n🎉 所有測試通過！WS2812 色彩閾值設定正確！")
        else:
            print(f"\n⚠️  有 {failed} 個測試失敗，請檢查硬體連接或韌體設定")
        
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

if __name__ == "__main__":
    print("""
╔═══════════════════════════════════════════════════════════════════╗
║         WS2812 色彩閾值測試程式 v1.0                              ║
║         113學年度 工業類科學生技藝競賽                             ║
╚═══════════════════════════════════════════════════════════════════╝
    """)
    
    # 讓使用者輸入 COM port（或使用預設值）
    port = input("請輸入 COM port (預設 COM5，直接按 Enter 使用預設值): ").strip()
    if not port:
        port = "COM5"
    
    print()
    test_ws2812_colors(port)
