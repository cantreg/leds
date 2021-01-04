# Leds
Arduino Gelid Codi6 programmable controller effects for ARGB 3pin 5v led strips and fans.
* Uses FastLed as a protocol implementation.
* Uses about a half of available flash memory ~ 16Kb.
* Uses somewhat less than **2048 bytes** of available memory for single effect (transition).
* A careful ```free()``` **is required** if you plan on adding your own effects.

# Defaults 
One 24 led long ARGB strip (2x daisy-chained Deepcool RGB 200PRO) in port 3.
```
#define NUM_LEDS 24
#define PORT_LED_1 3
```

1+2 ARGB fans (ZALMAN ZM-LF120) with 18 leds each in ports 5, 6, 9.
```
#define NUM_FAN 18
#define PORT_FAN_1 5
#define PORT_FAN_2 6
#define PORT_FAN_3 9
```

Fan effects are different from strips. Ports 6 and 9 acts synchronously on some transitions just because i'd liked it more.
