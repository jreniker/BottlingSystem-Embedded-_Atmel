# BottlingSystem-Embedded-_Atmel
Atmel based bottling system featuring six overflow nozzles for 8oz to 1gal bottles.

An individual bought a company that had bottled a proprietary product.  The bottling equipment involved the operator
manually engaging a pump to deliver product through overflow nozzles.  The operator would disengage the pump once
fluid was observed exiting the overflow for all six nozzles.  This mechanical process required a high degree of 
concentration, delivered inconsistent results requiring manual "top offs", was inefficient since the operator could
not multi-task staging new bottles, or moving the prior fill downline.

Experimentation demonstrated a time based fill with appropriately sized pumps delivered consistent results.
   > If bottling outside, the temperature will impact fill accuracy as plastic bottles may relax in the sun.
   > Bottles left in the sun will be filled inconsistently.
   > Centrifugal pumps will not deliver consistent fill.
   > Use a positive displacement pump such as a double diaphragm.  Do not oversize it.
   > Consider your pump displacement rate, and make sure you can consistently pressurize all nozzles.
   > Do not oversize your pump since it will pressurize your bottles too leading to an inconsistent fill.
   > An ideal setup may be to build a tank for your product above your bottling system, use a fill valve to
     maintain a constant level in your tank, but use gravity to fill all nozzles.
   > The pneumatic system is low pressure and uses a specific pneumatic cylinder with internal bumpers for slow
     open and close.  The pneumatic system also used small pilot air solenoid valves operated off of 12v or 120v
     with a controllable exhaust, actuated by the microcontroller.
   > This system was my own design based on how I thought a cylinder should operate, how a nozzle carrier should
     be built, and even drafted the designs for the controller enclosure and 3D printed it.

This is really maker project that was used in production, but the principals of process design, pneumatic system
design, control design, all make an appearance here.  Ultimately my "customer" was able to reduce production for the 
entire year of sales to a few weeks of work; all he had to do weekly was get the filled bottles off the shelf and ship.
Once per year, he replenished his stock - thanks to the process design, fabricated equipment, and bespoke controls.

Tips:  For hardware, use SSR's and not mechanical relays.  Noise in the system required the implementation of signal
"snubbers" as mechanical relays were energized.  The original hardware version used a high-current contactor, which
also generated noise significant enough to cause the platform to occasionally lose state and crash.  The contactor
was replaced by a high current solid state version.  Solid state relays are the way.  

The code is mine - if you use it, let me know!
