@echo off
REM 简单的编译脚本 - Tire模块
REM 用于在没有完整CMake配置的情况下编译tire模块

echo Compiling Tire Module...

REM 设置编译器
set CXX=cl.exe
set CXX_FLAGS=/c /std:c++17 /EHsc /W3 /I"..\third_party\eigen3" /I"." /DNOMINMAX /DWIN32_LEAN_AND_MEAN

REM 编译源文件
echo Compiling pacejka_model.cpp...
%CXX% %CXX_FLAGS% pacejka_model.cpp
if %errorlevel% neq 0 (
    echo Error compiling pacejka_model.cpp
    exit /b %errorlevel%
)

echo Compiling tire_dynamics.cpp...
%CXX% %CXX_FLAGS% tire_dynamics.cpp
if %errorlevel% neq 0 (
    echo Error compiling tire_dynamics.cpp
    exit /b %errorlevel%
)

REM 创建静态库
echo Creating tire.lib...
lib.exe /OUT:tire.lib pacejka_model.obj tire_dynamics.obj
if %errorlevel% neq 0 (
    echo Error creating tire.lib
    exit /b %errorlevel%
)

echo.
echo Tire module compiled successfully!
echo Library: tire.lib
echo.

REM 清理中间文件
del *.obj

pause
