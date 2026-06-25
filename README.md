# HeroEmu

Just a specialized IR remote for use in the Spider-Man W.E.B. Adventure dark ride at Disney Adventure World and Disney California Adventure. See https://furrtek.org/?a=wslinger for details.

Can be converted to a simple Flipper Zero IR file. I don't have one for testing so that's up to you.

Untested in the American version of the ride.

If you want to DIY:
 * Build code with Microchip Studio or use hex file directly.
 * Program MCU.
 * Connect button, 940nm IR LED, WS2812 LED, and battery according to pinout in header of `main.c`.
