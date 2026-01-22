@echo off
setlocal

echo ================================================
echo    Image Converter - Installation
echo ================================================
echo.

set "INSTALL_DIR=%LOCALAPPDATA%\ImageConverter"

echo Installing to: %INSTALL_DIR%
echo.

if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

echo Copying files...
xcopy /E /Y /I "%~dp0*" "%INSTALL_DIR%\" >nul 2>&1

echo Creating desktop shortcut...
powershell -Command " = New-Object -ComObject WScript.Shell;  = .CreateShortcut('%USERPROFILE%\Desktop\Image Converter.lnk'); .TargetPath = '%INSTALL_DIR%\image-converters.exe'; .WorkingDirectory = '%INSTALL_DIR%'; .Save()"

echo.
echo ================================================
echo    Installation Complete!
echo ================================================
echo.
echo Desktop shortcut created.
echo You can also run from: %INSTALL_DIR%\image-converters.exe
echo.
pause
