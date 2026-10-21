@echo off
rem Check if ADB is available
adb version >nul 2>&1
if %errorlevel% neq 0 (
    echo ADB Not installed correctly or not configured with environment variables, please check before running this script.
    pause
    exit /b 1
)
rem Use the ADB command to pull files to the current directory (i.e., the directory where the batch is executed)
adb pull /sdcard/SceneAnchorData.json
if %errorlevel% equ 0 (
    echo The file pull was successful.
) else (
    echo (File pulling failed, please check if the device connection and file path etc. are correct.
)
pause