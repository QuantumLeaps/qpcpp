set QPCPP=C:\qp\qpcpp
C:\qp\qtools\bin\g++ blinky.cpp -oblinky.exe -I%QPCPP%\include -I%QPCPP%\ports\win32 -L%QPCPP%\ports\win32\dbg -lqp