@echo off

set EngineVesion=5.3

for /f "skip=2 tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%EngineVesion%" /v "InstalledDirectory"') do set "EngineDirectory=%%b"

set AutomationToolPath="%EngineDirectory%\Engine\Build\BatchFiles\RunUAT.bat"
set PluginPath="%cd%\FabulousDualSense.uplugin"
set OutputPath="%cd%\Build"

title Build Plugin
echo Automation Tool Path: %AutomationToolPath%
echo:

call %AutomationToolPath% BuildPlugin -Plugin=%PluginPath% -Package=%OutputPath% -Rocket -TargetPlatforms=Win64
echo:
pause
exit 0
