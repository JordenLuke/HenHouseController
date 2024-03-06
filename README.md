This project ESP8266 project is to contorll a light and waterheating in my chicken coup. Though it may seam like a simple project the light control is a bit more
avanaced than a typical on off timer. The idea was to maximize the amount of light the chickens get but mimimize teh about of atrifical light. This proejct achives
this by taking advantage of the https://sunrise-sunset.org/api and "http://worldtimeapi.org/api/ip".

The light control works like this at mid night the 8266 gets the time for sunrise and sunet. Next it gets the current unix time. Once the controller knows what time it is,
it calaculaces how many milliseonnds till 6am sunrise sunset and 9pm. At six the light will come on and be on till Sunrise + 30min. The light will then come back on at 
sunset - 30min. The light will aslo trun on when the tempature inside the henhouse hit 10 f. It will then turn off when it hits 20f. 

The water temp is the same idea as the litght tempature control just shifted to trun on when the water is 33.0f and turn off 35.0f
