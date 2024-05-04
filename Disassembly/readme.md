# Disassembly

Although I have no plans to do any project with Terra Nova I do feel it's a game that needs some love and upgrading.

The major problem I had is that the exe for the game __ff.exe  (freefall was the original name for the game) obfuscates the gameplay program. Disassmlying __ff.exe only reveals a small unpacking program to extract the rest of the exe.

I have been able to extract a linear executable program file (__ff.le) from __ff.exe using an ancient utililty UNP.EXE 4.11 from 1995 written by a Ben Castricium and although I can open the file in IDA and disassemble it (__FF.idb and freefall.asm) I've had no real luck getting to grip with it's content. I suspect the data offsets to strings etc are off so I can't identify things like file loads, data structures etc. 

The linear exe does not run and appears to be missing a full header from what I can learn about linear exe file formats.

A bit of math and some hex editing will probably solve this problem...

Note: I've also tried pulling data using Dosbox Debuggers memory dump however I still have the same problem as above.

I've uploaded these files in case there is anyone out there who is looking into the same problem or can suggest a solution.