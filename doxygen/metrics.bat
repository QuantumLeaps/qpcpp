@setlocal

:: Simple complexity metrics tool  (adjust to your system) ...................
@set LIZARD=lizard

:: Generate metrics.dox file...
@set METRICS_INP=..\include ..\src -x ..\src\qs\*
@set METRICS_OUT=metrics.dox

@echo /** @page metrics Code Metrics > %METRICS_OUT%
@echo.>> %METRICS_OUT%
@echo @code{cpp} >> %METRICS_OUT%
@echo                    Code Metrics for QP/C++ >> %METRICS_OUT%

%LIZARD% -m -L500 -a10 -C20 -V %METRICS_INP% >> %METRICS_OUT%

@endlocal
