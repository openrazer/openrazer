<!-- : --- Self-Elevating Batch Script ---------------------------
@whoami /groups | find "S-1-16-12288" > nul && goto :admin
set "ELEVATE_CMDLINE=cd /d "%~dp0" & call "%~f0" %*"
cscript //nologo "%~f0?.wsf" //job:Elevate & exit /b

-->
<job id="Elevate"><script language="VBScript">
  Set objShell = CreateObject("Shell.Application")
  Set objWshShell = WScript.CreateObject("WScript.Shell")
  Set objWshProcessEnv = objWshShell.Environment("PROCESS")
  strCommandLine = Trim(objWshProcessEnv("ELEVATE_CMDLINE"))
  objShell.ShellExecute "cmd", "/c " & strCommandLine, "", "runas"
</script></job>
:admin -----------------------------------------------------------

@echo off
echo Running as elevated user.
echo Script file : %~f0
echo Arguments   : %*
echo Working dir : %cd%
echo.
copy System32\libusb0.dll %SystemRoot%\System32\
copy System32\libusbk.dll %SystemRoot%\System32\
copy System32\drivers\libusb0.sys %SystemRoot%\System32\drivers\
rem copy System32\drivers\libusbK.sys %SystemRoot%\System32\drivers\
copy SysWOW64\libusb0.dll %SystemRoot%\SysWOW64\
install-filter install -c=HIDClass
pause
	