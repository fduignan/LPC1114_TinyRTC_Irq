# LPC1114_TinyRTC
Sample code to interface LPC1114FN28 with the Tiny RTC board.
This version of the code makes use of deep sleep mode to save
power.  The CPU is woken up every second by the square wave output
from the TinyRTC.  Current falls to 67uA during deep sleep
