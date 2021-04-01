@echo off

:remove
@echo Dang xoa
@echo ========
FOR /F "tokens=*" %%C IN ('DIR ".\*.bak" /S /B') DO @echo Dang xoa "%%C" & DEL /F "%%C"
FOR /F "tokens=*" %%C IN ('DIR ".\*.scvd" /S /B') DO @echo Dang xoa "%%C" & DEL /F "%%C"
FOR /F "tokens=*" %%C IN ('DIR ".\*.uvgui.*" /S /B') DO @echo Dang xoa "%%C" & DEL /F "%%C"
goto clean

:clean
@echo.
@echo Nhan nut bat ky de clean toan bo project
pause >nul
@echo Don dep
@echo =======
FOR /F "tokens=*" %%D IN ('DIR ".\*.uvproj" /S /B') DO @echo Dang don dep "%%D" & "C:\Keil_v5\UV4\UV4" -c "%%D" -j0
goto rebuild

:rebuild
@echo.
@echo Nhan nut bat ky de rebuild toan bo project
pause >nul
@echo Build lai
@echo =========
FOR /F "tokens=*" %%E IN ('DIR ".\*.uvproj" /S /B') DO @echo Dang build lai "%%E" & "C:\Keil_v5\UV4\UV4" -b "%%E" -j0
goto exit

:exit
exit