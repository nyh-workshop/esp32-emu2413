## emu2413 for ESP32, beta 1.1, by nyh
---
## Source: https://github.com/digital-sound-antiques/emu2413
---

This is an attempt to have the emu2413 to run on ESP32. This is an experiment - it is playing some music using the Miditones. 

You have to use the Miditones converter with the commit f20fe9c: https://github.com/LenShustek/miditones/tree/f20fe9cb6d4d21e8440008c1d735f14867961b85 
because the later revisions do not parse the note stops.

There is a modification on this code (function OPLL_RateConv_getData in emu2413.c) where I have used this part of the code snippet from the source (https://gist.github.com/okaxaki/a601c0e1310833182b9c271aaadd1353#file-rate_conv-c-L73) 
to reduce the computation time by allowing fast rate conversion.

If this is not added, it results in stuttering of the audio. The ESP32 might struggle on very heavy computations - the code defaults to performing a sinc interpolation, which involve doubles and floats.

The output is already verified by ear with the executable that is compiled in emu2413.
