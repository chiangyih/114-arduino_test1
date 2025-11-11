"""
HC-05 藍牙連線功能測試程式
用於驗證 Connect to BLE 選單的資料接收功能
"""

import serial
import time
import sys

def test_ble_connection(port='COM5', baudrate=9600):
    """完整的藍牙連線功能測試"""
    
    try:
        # 連接序列埠
        print(f"正在連接到 {port} (Baud: {baudrate})...")
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # 等待 Arduino 重置
        print("✅ 連接成功！\n")
        
        print("=" * 70)
        print("  HC-05 藍牙連線功能測試")
        print("  請確認 Arduino 已進入 'Connect to BLE' 選單")
        print("=" * 70)
        print()
        
        input("準備好後按 Enter 開始測試...")
        print()
        
        test_results = []
        
        # 測試 1: CONNECT 命令
        print("【測試 1】發送 CONNECT 命令")
        print("→ 發送: CONNECT")
        ser.write(b"CONNECT\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 1 通過：CONNECT 命令成功")
            test_results.append(("CONNECT 命令", True))
        else:
            print("❌ 測試 1 失敗：未收到 ACK")
            test_results.append(("CONNECT 命令", False))
        
        print("→ 請確認 TFT 顯示 'Connected'（綠色）")
        print()
        time.sleep(2)
        
        # 測試 2: PING 命令
        print("【測試 2】發送 PING 命令（心跳）")
        print("→ 發送: PING")
        ser.write(b"PING\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 2 通過：PING 命令成功")
            test_results.append(("PING 命令", True))
        else:
            print("❌ 測試 2 失敗：未收到 ACK")
            test_results.append(("PING 命令", False))
        print()
        time.sleep(1)
        
        # 測試 3: LOAD 命令（綠色 - 30%）
        print("【測試 3】發送 LOAD 30（綠色，正常負載）")
        print("→ 發送: LOAD 30")
        ser.write(b"LOAD 30\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 3 通過：LOAD 30 命令成功")
            test_results.append(("LOAD 30 (綠色)", True))
        else:
            print("❌ 測試 3 失敗：未收到 ACK")
            test_results.append(("LOAD 30 (綠色)", False))
        
        print("→ 請確認 WS2812 顯示綠色（8 顆全亮）")
        print()
        time.sleep(3)
        
        # 測試 4: LOAD 命令（黃色 - 65%）
        print("【測試 4】發送 LOAD 65（黃色，中度負載）")
        print("→ 發送: LOAD 65")
        ser.write(b"LOAD 65\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 4 通過：LOAD 65 命令成功")
            test_results.append(("LOAD 65 (黃色)", True))
        else:
            print("❌ 測試 4 失敗：未收到 ACK")
            test_results.append(("LOAD 65 (黃色)", False))
        
        print("→ 請確認 WS2812 顯示黃色（8 顆全亮）")
        print()
        time.sleep(3)
        
        # 測試 5: LOAD 命令（紅色 - 90%）
        print("【測試 5】發送 LOAD 90（紅色，高負載）")
        print("→ 發送: LOAD 90")
        ser.write(b"LOAD 90\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 5 通過：LOAD 90 命令成功")
            test_results.append(("LOAD 90 (紅色)", True))
        else:
            print("❌ 測試 5 失敗：未收到 ACK")
            test_results.append(("LOAD 90 (紅色)", False))
        
        print("→ 請確認 WS2812 顯示紅色（8 顆全亮）")
        print()
        time.sleep(3)
        
        # 測試 6: LOAD 命令（邊界值測試）
        print("【測試 6】邊界值測試")
        boundary_tests = [
            (0, "綠色", "最小值"),
            (50, "綠色", "綠色上限"),
            (51, "黃色", "黃色下限"),
            (84, "黃色", "黃色上限"),
            (85, "紅色", "紅色下限"),
            (100, "紅色", "最大值")
        ]
        
        boundary_pass = 0
        for val, expected_color, desc in boundary_tests:
            print(f"  → LOAD {val:3d} ({desc}): ", end="")
            ser.write(f"LOAD {val}\n".encode())
            time.sleep(0.3)
            response = ser.readline().decode().strip()
            
            if response == "ACK":
                print(f"✅ ACK (預期: {expected_color})")
                boundary_pass += 1
            else:
                print(f"❌ 失敗")
            time.sleep(1.5)
        
        if boundary_pass == len(boundary_tests):
            print(f"✅ 測試 6 通過：所有邊界值測試成功 ({boundary_pass}/{len(boundary_tests)})")
            test_results.append(("邊界值測試", True))
        else:
            print(f"⚠️  測試 6 部分通過：{boundary_pass}/{len(boundary_tests)}")
            test_results.append(("邊界值測試", boundary_pass == len(boundary_tests)))
        print()
        time.sleep(2)
        
        # 測試 7: WRITE 命令
        print("【測試 7】發送 WRITE 123（寫入 EEPROM）")
        print("→ 發送: WRITE 123")
        ser.write(b"WRITE 123\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 7 通過：WRITE 123 命令成功")
            test_results.append(("WRITE 命令", True))
        else:
            print("❌ 測試 7 失敗：未收到 ACK")
            test_results.append(("WRITE 命令", False))
        
        print("→ 請切換到 EEPROM 選單確認數值為 123")
        print()
        time.sleep(2)
        
        # 測試 8: 持續心跳測試
        print("【測試 8】持續心跳測試（5 次 PING）")
        ping_success = 0
        for i in range(5):
            ser.write(b"PING\n")
            time.sleep(0.2)
            response = ser.readline().decode().strip()
            if response == "ACK":
                ping_success += 1
                print(f"  第 {i+1} 次 PING: ✅ ACK")
            else:
                print(f"  第 {i+1} 次 PING: ❌ 失敗")
            time.sleep(1)
        
        if ping_success == 5:
            print("✅ 測試 8 通過：心跳機制正常")
            test_results.append(("持續心跳", True))
        else:
            print(f"⚠️  測試 8 部分通過：{ping_success}/5")
            test_results.append(("持續心跳", ping_success == 5))
        
        print("→ TFT 應持續顯示 'Connected'")
        print()
        time.sleep(2)
        
        # 測試 9: DISCONNECT 命令
        print("【測試 9】發送 DISCONNECT 命令")
        print("→ 發送: DISCONNECT")
        ser.write(b"DISCONNECT\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 9 通過：DISCONNECT 命令成功")
            test_results.append(("DISCONNECT 命令", True))
        else:
            print("❌ 測試 9 失敗：未收到 ACK")
            test_results.append(("DISCONNECT 命令", False))
        
        print("→ 請確認 TFT 顯示 'Disconnect'（紅色）")
        print("→ WS2812 LED 應全部關閉")
        print()
        time.sleep(2)
        
        # 測試 10: 自動重新連線
        print("【測試 10】自動重新連線測試")
        print("→ 發送: PING（無需先 CONNECT）")
        ser.write(b"PING\n")
        time.sleep(0.3)
        response = ser.readline().decode().strip()
        print(f"← 回應: {response}")
        
        if response == "ACK":
            print("✅ 測試 10 通過：自動重新連線成功")
            test_results.append(("自動重新連線", True))
        else:
            print("❌ 測試 10 失敗：未收到 ACK")
            test_results.append(("自動重新連線", False))
        
        print("→ 請確認 TFT 自動變更為 'Connected'（綠色）")
        print()
        
        # 關閉序列埠
        ser.close()
        
        # 顯示測試摘要
        print("=" * 70)
        print("  測試摘要")
        print("=" * 70)
        print()
        
        passed = sum(1 for _, result in test_results if result)
        total = len(test_results)
        
        print(f"測試項目總數：{total}")
        print(f"通過項目：{passed}")
        print(f"失敗項目：{total - passed}")
        print(f"通過率：{passed/total*100:.1f}%")
        print()
        
        print("詳細結果：")
        for i, (test_name, result) in enumerate(test_results, 1):
            status = "✅ 通過" if result else "❌ 失敗"
            print(f"  {i:2d}. {test_name:20s} {status}")
        
        print()
        
        if passed == total:
            print("🎉 所有測試通過！Connect to BLE 功能完全正常！")
        elif passed >= total * 0.8:
            print("⚠️  大部分測試通過，但有少數項目需要檢查")
        else:
            print("❌ 測試失敗較多，請檢查硬體連接和程式碼")
        
    except serial.SerialException as e:
        print(f"❌ 序列埠錯誤: {e}")
        print(f"\n請確認：")
        print(f"1. Arduino 已正確連接到 {port}")
        print(f"2. HC-05 藍牙模組已正確配對")
        print(f"3. 序列埠未被其他程式佔用")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n\n⚠️  使用者中斷測試")
        if 'ser' in locals() and ser.is_open:
            ser.close()
        sys.exit(0)
    except Exception as e:
        print(f"❌ 未預期的錯誤: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    print("""
╔═══════════════════════════════════════════════════════════════════╗
║         HC-05 藍牙連線功能測試程式 v1.0                           ║
║         Connect to BLE 選單資料接收驗證                            ║
║         113學年度 工業類科學生技藝競賽                             ║
╚═══════════════════════════════════════════════════════════════════╝
    """)
    
    print("測試前準備：")
    print("1. 確認 Arduino 已上傳最新韌體")
    print("2. 確認 HC-05 已正確接線（TX→RX, RX→TX）")
    print("3. 確認 PC 端已與 HC-05 配對")
    print("4. 確認 Arduino 已進入 'Connect to BLE' 選單")
    print()
    
    port = input("請輸入 COM port (預設 COM5): ").strip()
    if not port:
        port = "COM5"
    
    print()
    test_ble_connection(port)
