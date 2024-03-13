This project, ESP8266, is to control the light and water heating in my chicken coup. Though it may seem like a simple project, the light control is a bit more
advanced than that of a typical on-off timer. The idea was to maximize the amount of light the chickens get but minimize the amount of artificial light. This project achieves
this using the "https://sunrise-sunset.org/api" and "http://worldtimeapi.org/api/ip."

The light control works like this at midnight. The 8266 gets the time for sunrise and sunset. Next, it requests the current Unix time. Once the controller knows the time,
it calculates how many milliseconds it will be until 6 am, sunrise, sunset, and 9 pm. At six, the light will come on until Sunrise + 30min. The light will then come back on at 
sunset - 30min. The light will also turn on when the temperature inside the henhouse hits ten f. It will then turn off when it hits 20f. 

The water temp is the same idea as the light temperature control just shifted to turn on when the water is 33.0f and turn off at 35.0f
