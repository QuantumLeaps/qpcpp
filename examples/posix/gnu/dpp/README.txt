The Makefile provided with this project assumes that the 
environment variable QPCPP is defined and it points to the
location of the QP/C++ framework. For example, assuming that
you have installed QP/C++ into the directory ~/qpc,
you should define the environment variable QPCPP to 
~/qpcpp.
 
Editing the Environment Variables on Linux
------------------------------------------
You need to edit the ~/.bash_profile file to add the
following line:

export QPCPP=~/qpcpp

assuming that you have installed QP/C++ into ~/qpcpp.

****
NOTE: After updating the ~/.bash_profile file, you shold log off
your account and then log on again so that all applications can
pick up the changes.
****