Use the EGU to trigger an event from Software, use this event to toggle an LED
No SDK code is used, everything is defined manually.

Writing to the task register in the EGU triggers an event which then triggers an IRQ. 
In the IRQ handler, the LED is toggled.

In the main loop, a simple delay (counting down from a value) is used to trigger the task periodically.