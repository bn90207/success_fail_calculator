# success_fail_calculator

功能簡介: 每日統計CAM PC、Fuji曝光機自動轉檔成功率，並提供各筆失敗原因(Error code)。同時為彌補程式遭關閉，導致部分天數遺漏，另外提供(1)remove: 從統計結果中刪除特定日期；(2)insert: 重新計算指定日期的結果，並插入至正確位置。

## 1.success_fail_calculator執行原理

### 1-1.CAM PC成功率統計

CAM PC由CiMES接收轉檔需求後，會將輸出結果至於下列兩種路徑:

1. "\\\\10.1.11.73\\di_mac_mes\\finish": 轉檔成功
2. "\\\\10.1.11.73\\di_mac_mes\\ng\\error_code": 當轉檔失敗時，CAM PC會依失敗原因於"\\\\10.1.11.73\\di_mac_mes\\ng\\"路徑下創建資料夾，並將結果至於此處。因此蒐集失敗率時需遍歷"\\\\10.1.11.73\\di_mac_mes\\ng\\"下的所有資料夾，並以資料夾名作為error code。

另外由於文件內"EXPORT_DATE = "標明轉檔日期，所以可區分出不同日期的成功/失敗筆數。

### 1-1.Fuji曝光機成功率統計

類似CAM PC，Fuji曝光機將轉檔結果分別至於:

1. "\\\\10.1.11.73\\di\\FUJI\\camdata\\register_ok\\設備編號": 轉檔成功
2. "\\\\10.1.11.73\\di\\FUJI\\camdata\\register_error\\設備編號": 轉檔失敗，檔案內首行為失敗原因。

同時檔案名稱前8碼包含西元年\月\日的執行時間，所以同樣區別出不同日期的結果。
