@echo off
REM Scheduler模块独立编译脚本
REM 用于测试scheduler模块是否可以正确编译

echo ========================================
echo Scheduler Module Compilation Test
echo ========================================

REM 设置编译器（使用MSVC）
set CXX=cl.exe
set CXXFLAGS=/std:c++17 /EHsc /W4 /WX /I..\src /I..\src\scheduler /DWIN32 /D_WINDOWS

REM 编译源文件
echo.
echo Compiling realtime_scheduler.cpp...
%CXX% %CXXFLAGS% /c ..\src\scheduler\realtime_scheduler.cpp /Fo:realtime_scheduler.obj
if %ERRORLEVEL% NEQ 0 (
    echo FAILED to compile realtime_scheduler.cpp
    exit /b 1
)
echo SUCCESS: realtime_scheduler.cpp compiled

echo.
echo Compiling ipc_manager.cpp...
%CXX% %CXXFLAGS% /c ..\src\scheduler\ipc_manager.cpp /Fo:ipc_manager.obj
if %ERRORLEVEL% NEQ 0 (
    echo FAILED to compile ipc_manager.cpp
    exit /b 1
)
echo SUCCESS: ipc_manager.cpp compiled

REM 创建静态库
echo.
echo Creating static library...
lib.exe /OUT:ev_scheduler.lib realtime_scheduler.obj ipc_manager.obj
if %ERRORLEVEL% NEQ 0 (
    echo FAILED to create static library
    exit /b 1
)
echo SUCCESS: ev_scheduler.lib created

echo.
echo ========================================
echo Compilation Test PASSED
echo ========================================
echo.
echo Statistics:
echo - realtime_scheduler.obj created
echo - ipc_manager.obj created
echo - ev_scheduler.lib created
echo.

exit /b 0
