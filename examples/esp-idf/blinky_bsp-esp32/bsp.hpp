#ifndef BSP_HPP
#define BSP_HPP

class BSP {
public:
    enum { TICKS_PER_SEC = CONFIG_FREERTOS_HZ } ;
    static void init(void);
    static void ledOff(void);
    static void ledOn(void);
};

#endif // BSP_HPP
