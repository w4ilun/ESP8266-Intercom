# ESP8266 Intercom

![Fritzing](Fritzing.png)

An ESP8266 is connected to a 2-channel relay module.
* Channel 1 controls the door, with the relay closed for 2 seconds only
* Channel 2 controls the buzzer, it polls a time server every 30 seconds to get the current time. The buzzer is set to on or off based on the "start time" and "end time"
