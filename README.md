# Alarm-Detector

BIG CHANGE:
Not doing ESP-32. Low clock speed + sleep leads to disconnections on the wifi or the
wifi not connecting at all. Therefore, quick pivot into Raspberry Pi Zero. Pretty
much just a Rpi that runs off of an SD card. Has the GPIO capabilities of ESP.

Same as before:

With a properly configured home network with home assistant OS running on a mini PC with a Zigbee stick, you can trigger the lightbulbs or light switch in your room using a Raspberry Pi Zero sending an on signal to Home Assistant OS using the Zigbee protocol and then triggering the light using it.

The harder part is going to be switching on the bed shaker. The bedshaker uses a usb power brick to turn on. The hard method would be to use a contactor that triggers when the Raspberry Pi Zero detects an alarm and that contactor closes the loop between the USB cable coming out of the usb power brick and the bedshaker. The easy method would be to have the Raspberry Pi Zero send an on switch to the Home Assistant OS using the Zigbee protocol and then trigger a zigbee wall plug to turn on the USB brick which turns on the bedshaker. Nice?

TODO: throw up circuit diagrams and documentation.
