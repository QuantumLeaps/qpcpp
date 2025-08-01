@setlocal

set VERSION=8.0.5

:: usage
@echo Usage: qppcp_sha1 [gen]
@echo examples:
@echo qppcp_sha1     : check the sha1 sums in the file qpcpp_%VERSION%.sha1
@echo qpcpp_sha1 gen : generate the sha1 file qpcpp_%VERSION%.sha1
@echo.

@if NOT "%1"=="gen" (
sha1sum --check --warn qpcpp_%VERSION%.sha1
goto end
)

@echo generating qpcpp_%VERSION%.sha1...
@sha1sum ^
    include/* ^
    src/qf/* src/qk/* src/qs/* src/qv/* src/qxk/* ^
    ports/arm-cm/config/* ^
    ports/arm-cm/qk/armclang/* ports/arm-cm/qk/gnu/* ports/arm-cm/qk/iar/* ^
    ports/arm-cm/qv/armclang/* ports/arm-cm/qv/gnu/* ports/arm-cm/qv/iar/* ^
    ports/arm-cm/qxk/armclang/* ports/arm-cm/qxk/gnu/* ports/arm-cm/qxk/iar/* ^
    ports/arm-cm/qutest/* ^
    ports/arm-cr/config/* ^
    ports/arm-cr/qk/gnu/* ports/arm-cr/qk/iar/* ports/arm-cr/qk/ti/* ^
    ports/arm-cr/qv/gnu/* ports/arm-cr/qv/iar/* ports/arm-cr/qv/ti/* ^
    ports/msp430/qk/* ports/msp430/qv/* ports/msp430/qutest/* ^
    ports/config/* ^
    ports/embos/* ^
    ports/freertos/* ^
    ports/threadx/* ^
    ports/uc-os2/* ^
    ports/qep-only/* ^
    ports/posix/* ports/posix-qv/* ports/posix-qutest/* ^
    ports/win32/* ports/win32-qv/* ports/win32-qutest/* ^
    zephyr/* ^
    > qpcpp_%VERSION%.sha1
qclean
@echo done

:end
@endlocal
