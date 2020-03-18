:: Generate Metrics for QP/C++ ..............................................
@set LIZARD=C:\tools\Python27\python.exe C:\tools\lizard\lizard.py 

@set METRICS_INP=..\include ..\src -x ..\src\qs\*
@set METRICS_OUT=metrics.dox

@echo /** @page metrics Code Metrics > %METRICS_OUT%
@echo.>> %METRICS_OUT%
@echo @code{cpp} >> %METRICS_OUT%
@echo                    Code Metrics for QP/C++ >> %METRICS_OUT%

%LIZARD% -m -L500 -a10 -C20 -V %METRICS_INP% >> %METRICS_OUT%

@echo @endcode >> %METRICS_OUT%
@echo */ >> %METRICS_OUT%
