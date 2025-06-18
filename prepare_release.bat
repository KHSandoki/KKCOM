@echo off
echo ========================================
echo KKCOM v1.0.0-beta.1 Release Preparation
echo ========================================
echo.

echo [1/4] Building C++ Edition...
cd KKCOM_CPP
call build_vcpkg.bat
if errorlevel 1 (
    echo ERROR: C++ build failed!
    pause
    exit /b 1
)

echo.
echo [2/4] Copying executable to release folder...
if not exist "..\release" mkdir "..\release"
if exist "build_vcpkg\Release\KKCOM_CPP.exe" (
    copy "build_vcpkg\Release\KKCOM_CPP.exe" "..\release\KKCOM-Windows-v1.0.0-beta.1.exe"
    echo C++ executable copied successfully!
) else (
    echo WARNING: C++ executable not found!
)

cd ..

echo.
echo [3/4] Creating Python package...
if not exist "release" mkdir "release"
7z a -tzip "release\KKCOM-Python-v1.0.0-beta.1.zip" "main.py" "README.md" "LICENSE" "CLAUDE.md" "icon_*.svg" "README_ICONS.md"
if errorlevel 1 (
    echo WARNING: 7z not found, creating manual zip...
    echo Please manually zip: main.py, README.md, LICENSE, CLAUDE.md, icon files
) else (
    echo Python package created successfully!
)

echo.
echo [4/4] Creating source archive...
7z a -tzip "release\KKCOM-Source-v1.0.0-beta.1.zip" "*" -x!release -x!KKCOM_CPP\build_* -x!KKCOM_CPP\vcpkg -x!KKCOM_CPP\libs -x!"*.log" -x!"*.tmp" -x!".git*"
if errorlevel 1 (
    echo WARNING: Source archive creation failed
) else (
    echo Source archive created successfully!
)

echo.
echo ========================================
echo Release preparation completed!
echo ========================================
echo.
echo Files created in 'release' folder:
if exist "release\KKCOM-Windows-v1.0.0-beta.1.exe" echo - KKCOM-Windows-v1.0.0-beta.1.exe
if exist "release\KKCOM-Python-v1.0.0-beta.1.zip" echo - KKCOM-Python-v1.0.0-beta.1.zip
if exist "release\KKCOM-Source-v1.0.0-beta.1.zip" echo - KKCOM-Source-v1.0.0-beta.1.zip
echo.
echo Next steps:
echo 1. Test the executable
echo 2. Commit and push to GitHub
echo 3. Create release on GitHub with these files
echo 4. Update download links in README.md
echo.
pause