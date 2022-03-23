# DPP Demo for ESP32 SDK v4.4

Dining philosophers problem demo for ESP32. 

## Requirements

[ESP-IDF v4.4](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)

## Build and test

```bash
python ESP_TOOLS_DIR/idf.py build
python ESP_TOOLS_DIR/idf.py flash monitor -p /dev/ttyUSB0
```

### Output

```
I (1268) table: Philo0 thinking
I (1271) table: Philo1 thinking
I (1275) table: Philo2 thinking
I (1279) table: Philo3 thinking
I (1282) table: Philo4 thinking
I (1990) table: Philo3 hungry
I (1990) table: Philo3 eating
I (2198) table: Philo2 hungry
I (2251) table: Philo0 hungry
I (2251) table: Philo0 eating
I (2512) table: Philo1 hungry
I (2699) table: Philo4 hungry
I (3169) table: Philo3 thinking
I (3169) table: Philo2 eating
I (4203) table: Philo0 thinking
I (4203) table: Philo4 eating
I (4401) table: Philo3 hungry
I (4493) table: Philo2 thinking
```

