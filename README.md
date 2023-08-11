# BIIS
Text based OS for Arduino platform

# Copmponents:
- 1x Arduino (Arduino MEGA is recomended)
- 1x LCD Display (With I2C )
- 1x Button
- 1x DHT sensor
- 1x Buzzer
- 1x 10K ohm resistor

# Libraries:
- stdlib.h
- dht.h
- stdio.h
- Wire.h
- LiquidCrystal_I2C.h
- string.h
- EEPROM.h
- Time.h
- TimeLib.h
- math.h

# Help:
- To submit morse code sequence or make space, execute long press (~500ms)
- To delete last character write 4 dashes (----)
- To exit program, press and hold button for a long time (~1000ms)
- If you cannot write in morse code use "Morse code.txt" to help you

## Timing:
- Dot (.) = ~<100ms
- Dash (-) = ~>100ms
- Long press (LONG) = ~>1000ms

## Commands and programs:
- TEMP; (-/./--/.--./-.-.-.)
  - Show temperature and humidity
- SW; (.../.--/-.-.-.)
  - Stopwatch
- ABOUT; (.-/-.../---/..-/-/-.-.-.)
  - Show BIIS version
- HELP; (...././.-../.--./-.-.-.)
  - Show all commands
- PLAY /Buzzer frequency/; (.--./.-../.-/-.--/LONG/BUZZER FREQUENCY/-.-.-.)
  - Play noise with set frequency
- DICE; (-../../-.-././-.-.-.)
  - Roll a dice
- RNG /Max number/; (.-./-./--./LONG/MAX NUMBER/-.-.-.)
  - Generates random number between 0 and max number
- STIMER /Seconds/; (.../-/../--/./.-./LONG/SECONDS/-.-.-.)
  - Set timer for set number of seconds
- MTIMER /Minutes/; (--/-/../--/./.-./LONG/MINUTES/-.-.-.)
  - Set timer for set number of minutes
- COM /Num1/ /+-/*V^/ /Num2/; (-.-./---/--/LONG/num1/op/num2/-.-.-.)
  - Do computations with two numbers
- OFF; (---/..-./..-./-.-.-.)
  - Shutdown BIIS
- SLEEP; (.../.-../././.--./-.-.-.)
  - Turn off the display
- RESTART; (.-././.../-/.-/.-./-/-.-.-.)
  - Restart BIIS

## User settings:
- CURSOR; (-.-./..-/.-./.../---/.-./-.-.-.)
  - Toggle cursor
- BUZZER; (-.../..-/--../--.././.-./-.-.-.)
  - Toggle buzzer
- SHELP; (.../...././.-../.--./-.-.-.)
  - Toggle help, that shows at start
- BKLIGHT; (-.../-.-/.-../../--./..../-/-.-.-.)
  - Toggle display backlight
- RESET; (.-././..././-/-.-.-.)
  - Reset all settings
