# KKCOM 獨立執行檔構建指南

## 🎯 目標
將KKCOM C++版本構建為單一執行檔，無需額外的DLL文件（如glfw3.dll）。

## 🔧 實現方法

### 方法1: 靜態鏈接 (推薦)

#### 步驟1: 安裝靜態依賴庫
```bash
# 運行設置腳本 (已修改為安裝靜態版本)
setup_vcpkg.bat

# 或手動安裝
cd vcpkg
vcpkg install glfw3:x64-windows-static
vcpkg install nlohmann-json:x64-windows-static
```

#### 步驟2: 使用獨立構建腳本
```bash
# 一鍵構建獨立執行檔
build_standalone.bat
```

#### 步驟3: 驗證結果
構建完成後檢查：
- 執行檔大小應該增加（包含了靜態庫）
- 不再需要glfw3.dll文件
- 可以在沒有安裝GLFW的機器上運行

### 方法2: DLL嵌入 (複雜)

#### 使用資源嵌入
```cpp
// 在main.cpp中添加資源嵌入代碼
#include <windows.h>

bool ExtractEmbeddedDLL() {
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_GLFW_DLL), RT_RCDATA);
    if (!hResource) return false;
    
    HGLOBAL hMemory = LoadResource(NULL, hResource);
    if (!hMemory) return false;
    
    DWORD dwSize = SizeofResource(NULL, hResource);
    LPVOID lpData = LockResource(hMemory);
    
    // 寫入臨時文件
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    strcat(tempPath, "glfw3.dll");
    
    HANDLE hFile = CreateFile(tempPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    
    DWORD written;
    WriteFile(hFile, lpData, dwSize, &written, NULL);
    CloseHandle(hFile);
    
    // 載入DLL
    LoadLibrary(tempPath);
    return true;
}
```

## 📊 比較分析

### 靜態鏈接 vs 動態鏈接

| 特性 | 靜態鏈接 | 動態鏈接 |
|------|----------|----------|
| **檔案大小** | 較大 (~5-8MB) | 較小 (~2MB + DLL) |
| **部署便利性** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| **記憶體使用** | 較高 | 較低 |
| **啟動速度** | 較快 | 稍慢 |
| **更新便利性** | 需重新編譯 | 可單獨更新DLL |
| **相容性** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |

### 建議選擇

**推薦使用靜態鏈接，因為：**
1. **部署簡單** - 只需一個exe文件
2. **相容性好** - 不依賴系統DLL版本
3. **用戶體驗佳** - 下載即用，無需安裝
4. **檔案大小可接受** - 現代標準下5-8MB很小

## 🛠️ 構建配置

### CMakeLists.txt 關鍵設置
```cmake
# 強制靜態鏈接 (Windows)
if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(BUILD_SHARED_LIBS OFF)
    # 使用靜態運行時庫
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()
```

### vcpkg 配置
```bash
# 使用靜態三元組
-DVCPKG_TARGET_TRIPLET=x64-windows-static
```

## 🚀 自動化構建

### build_standalone.bat 特點
1. **自動檢測** - 檢查依賴是否已安裝
2. **自動安裝** - 自動安裝缺失的靜態庫
3. **清理構建** - 每次重新構建確保乾淨
4. **多VS版本支持** - 支持VS2019和VS2022
5. **結果驗證** - 構建後自動檢查

### 使用方法
```bash
# 一鍵構建
cd KKCOM_CPP
build_standalone.bat

# 結果在 build_standalone\Release\KKCOM_CPP.exe
```

## ⚠️ 注意事項

### 1. Visual C++ Runtime
確保目標機器有Visual C++ Runtime，或使用靜態Runtime：
```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
```

### 2. OpenGL驅動
應用程式仍需要：
- 基本OpenGL支持（Windows內建）
- 顯卡驅動程式

### 3. 檔案大小
靜態鏈接後檔案大小約：
- **Debug版本**: 15-20MB
- **Release版本**: 5-8MB
- **Release + 優化**: 3-5MB

### 4. 編譯時間
靜態鏈接會增加編譯時間：
- 首次編譯：+2-3分鐘
- 增量編譯：影響較小

## 🎉 最終效果

成功構建後獲得：
- ✅ 單一執行檔 (KKCOM_CPP.exe)
- ✅ 無DLL依賴
- ✅ 可在任何Windows 10/11機器運行
- ✅ 下載即用的用戶體驗

這樣的獨立執行檔非常適合分發和部署，用戶無需安裝任何額外軟體即可運行KKCOM。