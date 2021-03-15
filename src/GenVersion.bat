@echo off

setlocal

set HEADER_NAME="%~dp0generated_git_version.h"

for /f %%i in ('git describe --tags --dirty --long') do set GIT_VERSION=%%i
set FULL_STRING=#define GIT_VERSION_DESC  "%GIT_VERSION%"
if exist %HEADER_NAME% (
    set /p CURRENT_STRING=<%HEADER_NAME%
) else (
    set CURRENT_STRING=_does_not_exist_
)
if "!%FULL_STRING%!" == "!%CURRENT_STRING%!" goto DONE


for /F "tokens=1,2,3,4,5,6 delims=.-" %%a in ("%GIT_VERSION%") do (
    set GIT_MAJOR=%%a
    set GIT_MINOR=%%b
    set GIT_PATCH=%%c
    set GIT_REV=%%d
    set GIT_HASH=%%e
    if [%%f] == [] (
        set GIT_DIRTY=
        set GIT_DIRTY_STR=""
    ) else (
        set GIT_DIRTY=*
        set GIT_DIRTY_STR="*"
    )
)

(echo %FULL_STRING%)> %HEADER_NAME%
(echo #define GIT_VERSION_MAJOR %GIT_MAJOR%)>> %HEADER_NAME%
(echo #define GIT_VERSION_MINOR %GIT_MINOR%)>> %HEADER_NAME%
(echo #define GIT_VERSION_PATCH %GIT_PATCH%)>> %HEADER_NAME%
(echo #define GIT_VERSION_REV   %GIT_REV%)>> %HEADER_NAME%
(echo #define GIT_VERSION_DIRTY %GIT_DIRTY_STR%)>> %HEADER_NAME%
(echo #define GIT_VERSION_HASH  "%GIT_HASH%")>> %HEADER_NAME%

echo Version: %GIT_MAJOR%.%GIT_MINOR%.%GIT_PATCH%.%GIT_REV%-%GIT_HASH%%GIT_DIRTY%

:DONE

endlocal
