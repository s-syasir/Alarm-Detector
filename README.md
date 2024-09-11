# Alarm-Detector
ESP 32 C project that uses a photo resistor to detect my alarm clock going off to then trigger a bed shaker and turn on my lights.

With a properly configured home network with home assistant OS running on a mini PC with a Zigbee stick, you can trigger the lightbulbs or light switch in your room using an ESP-32 sending an on switch to the
Home Assistant OS using the Zigbee protocol and then triggering the light using it.

The harder part is going to be switching on the bed shaker. The bedshaker uses a usb power brick to turn on. The hard method would be to use a contactor that triggers when the ESP-32 detects an alarm and that contactor closes the loop between the USB cable coming out of the usb power brick and the bedshaker. The easy method would be to have the ESP-32 send an on switch to the Home Assistant OS using the Zigbee protocol and then trigger a zigbee wall plug to turn on the USB brick which turns on the bedshaker. Nice?

TODO: throw up circuit diagrams and documentation.
