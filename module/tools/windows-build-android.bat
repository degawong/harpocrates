@echo off

REM set "BUILD_SHARED_LIBS=OFF"
REM if "%1" == "so" set "BUILD_SHARED_LIBS=ON"

REM set "ABI=arm64-v8a"
REM if "%2" == "x64" set "ABI=arm64-v8a"
REM if "%2" == "x32" set "ABI=armeabi-v7a"

REM set "BUILD_TYPE=%3"
REM if "%2" == "debug" set "BUILD_TYPE=Debug"
REM if "%2" == "release" set "BUILD_TYPE=Release"

set "ABI=armeabi-v7a"
set "BUILD_TYPE=Release"
set "BUILD_SHARED_LIBS=OFF"

set ANDROID_NDK=D:\android-ndk-r17b

cmake -G"MinGW Makefiles" ^
	-DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%\build\cmake\android.toolchain.cmake ^
	-DANDROID_TOOLCHAIN=clang ^
	-DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
	-DBUILD_SHARED_LIBS=%BUILD_SHARED_LIBS% ^
	-DANDROID_ABI="%ABI%" ^
	-DANDROID_NATIVE_API_LEVEL=android-21 ^
	-DCMAKE_MAKE_PROGRAM="%ANDROID_NDK%\prebuilt\windows-x86_64\bin\make.exe" ../
%ANDROID_NDK%\prebuilt\windows-x86_64\bin\make -j 8

set "ABI=arm64-v8a"

cmake -G"MinGW Makefiles" ^
	-DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%\build\cmake\android.toolchain.cmake ^
	-DANDROID_TOOLCHAIN=clang ^
	-DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
	-DBUILD_SHARED_LIBS=%BUILD_SHARED_LIBS% ^
	-DANDROID_ABI="%ABI%" ^
	-DANDROID_NATIVE_API_LEVEL=android-21 ^
	-DCMAKE_MAKE_PROGRAM="%ANDROID_NDK%\prebuilt\windows-x86_64\bin\make.exe" ../
%ANDROID_NDK%\prebuilt\windows-x86_64\bin\make -j 8

@echo on
