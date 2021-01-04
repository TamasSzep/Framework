del /q /f "..\Build\Resources\*.*"
for /d %%i in ("..\Build\Resources\*") do rd /s /q "%%i"