The Makefile provided with this project assumes that the 
environment variable QPCPP is defined and it points to the
location of the QP/C++ framework. For example, assuming that
you have installed QP/C++ into the directory ~/qpc,
you should define the environment variable QPCPP to 
~/qpcpp.
 
Additionally, to build the SPY configuration, the Qtools
collectin must be also istalled on your machine and the
environment variable QTOOLS has been defined.

Editing the Environment Variables on Linux
------------------------------------------
You need to edit the ~/.bash_profile file to add the
following line:

export QPCPP=~/qpcpp
export QTOOLS=~/qtools

assuming that you have installed QP/C++ into ~/qpcpp and
Qtools into ~/qtools.


Editing the Environment Variables on Mac OS X
---------------------------------------------
You need to edit the ~/.profile file to add the
following line:

export QPCPP=~/qpcpp
export QTOOLS=~/qtools

assuming that you have installed QP/C into ~/qpc and
Qtools into ~/qtools.


****
NOTE: After updating the ~/.bash_profile file, you shold log off
your account and then log on again so that all applications can
pick up the changes.
****