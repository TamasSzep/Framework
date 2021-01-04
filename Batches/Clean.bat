del /q /f "..\Build\*.*"
for /d %%i in ("..\Build\*") do rd /s /q "%%i"
del /q /f "..\Temp\*.*"
for /d %%i in ("..\Temp\*") do rd /s /q "%%i"