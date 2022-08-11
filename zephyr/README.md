# qpcpp Zephyr Module
This directory defines the
[QP/C++ Real-Time Embedded Framework](https://github.com/QuantumLeaps/qpcpp)
as a [Zephyr module](https://docs.zephyrproject.org/latest/develop/modules.html).

# How to use
Example of use is provided in the related repository
[qpcpp-zephyr-app](https://github.com/QuantumLeaps/qpcpp-zephyr-app)


## Configuring the QPCPP Zephyr Module
The `Kconfig` file provides configuration `CONFIG_QPCPP` to activate
the QPCPP module in Zephyr. To do so, you need to add the following
line to your `prj.conf`:

```ini
CONFIG_QPCPP=y
```

## Configuring the QSPY Software Tracing
If you wish to enable
[QSPY Software Tracing](https://www.state-machine.com/qtools/qpspy.html),
`Kconfig` file provides configuration `CONFIG_QSPY`, which you can
use in your `prj.conf`:

```ini
CONFIG_QSPY=y
```
