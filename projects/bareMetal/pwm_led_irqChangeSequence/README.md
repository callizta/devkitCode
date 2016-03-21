This is a simple example how to fade LEDs using the PWM
No SDK code is used, everything is defined manually.

In total, three LEDs are used:
* STATUS_LED, switched on as soon as program is running
* PWM_LED, controlled by the output of PWM0
* PWM_IRQ_LED, toggled whenever the IRQ is fired (either by seq0 done or seq1 done. loopsdone doesn't toggle PWM_IRQ_LED)

The program fades in and out PWM_LED, and after each loop, the brightness of PWM_LED is reduced a bit. 
When the brightness is almost 0, it's reset to it's maximum value again.

The update of the brightness values (i.e. update the PWM0 buffer) is done in the IRQ handler, i.e. the processor is sleeping most of the time
