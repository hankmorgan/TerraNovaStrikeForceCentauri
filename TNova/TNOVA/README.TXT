       TERRA NOVA: STRIKE FORCE CENTAURI

               README.TXT FILE

====================================================

 1) If you're installing in Windows95 mode, and
 the installer hangs after restarting in DOS,
 you may not have a DOS mouse driver.  Check
 to see if you have a driver and/or contact
 your hardware provider.
    If you don't have DOS video or sound 
 drivers, you will also have problems running
 Terra Nova.  Contact your hardware provider.

 2) If you have a Matrix MGA Millenium, run
 Terra Nova WITHOUT INSTALLING UNIVBE and with 
 the +BADPALDAC switch.

 3) A sound card is required to play Terra Nova,
 but the game will run without one.  You won't
 be able to hear your squad speak or understand
 any of the movies.

 4) If you select General MIDI in the Choose
 New Card menu, the display in the lower left
 will read "Roland SCC1".  Don't worry.  This
 is supposed to happen.

 5) The swap file space listed in your install
 size must always be free in order to play
 Terra Nova.  For instance, Minimal install
 requires a 20 Mb Swap on your hard drive.  If 
 you select Minimal install, you must have 20 Mb 
 free on your hard drive whenever you play Terra 
 Nova.

 6) Hires mode (320x400) requires additional 
 memory, so you cannot select it in the Options 
 panel if you only have 8 megabytes of memory.  
 However, you may be able to successfully use it 
 on many missions, depending on exactly how much 
 memory you have free.  If you want to try, you 
 can enable hires on 8 megabyte machines by 
 running "tn +hires", and then switching to 
 320x400 mode in the Options panel.

 7) Projectile weapons (such as the grenade 
 launcher and the missile launcher) have 
 ammunition in "clips" of 3 shells per clip.  The 
 reload time for the first three shots will be 
 short, followed by a longer reload time as a new 
 clip loads into the weapon.

 8) Projectile weapon ranges are based upon 
 normal gravity and even elevation, and may
 vary in practice.

 9) Keep moving!  A moving target is harder to
 hit than a stationary one.

 10) The Alt-S hotkey commands your squad to
 give a Status check.  They will tell you their
 current condition.

 11) If you want to replay one of the cutscenes 
 that appear as you leave your quarters, go back 
 into your quarters, save the game, and reload it. 
 Then when you leave your quarters the scene will 
 show again.

 12) Your PBA has an internal voice monitor which 
 will verbally inform you of significant events: 
 systems damage, drone launch, ASF activated, etc.

 13) If your squadmates keep cutting into your 
 line of fire, it's often because they're trying 
 to stay in formation.  By sending them an 
 Aggressive Attack command (Alt-A will send this 
 to your whole squad) they'll give up the 
 formation, thus alleviating this problem.

 14) Mixing alcaps and water makes a great 
 imitation martini.



            - Team Schmitty
           (the TN Developers)




====================================================


TECHNICAL NOTES

Terra Nova does not officially support VR 
headsets or joysticks.  However, you should be
able to get them running:  




VR HEADSETS
-----------

First, you must have the headset turned on before 
you start up Terra Nova.

VR headset support is enabled through command 
line arguments.  For example, you would type 
"tn +vr" rather than typing "tn".

   +vr       enables headset auto-detection.  The 
             game should auto-detect which type of 
             headset you have.
    
   +iglass   only looks for the Virtual IO iglasses
   +vfx1     only looks for the Forte VFX1 Headgear
   +cmaxx    only looks for the VictorMaxx CyberMaxx    

These put you into a mode where the direction you 
look is the direction your PBA faces.  That way, 
if your head turns, your PBA turns to match.  
You'll still need to use the keyboard to turn as 
well, since your head only moves so far.  The [F] 
key and keypad center key recenter your pitch and 
heading to zero.

The more advanced (but harder) way to play is by 
also specifying "+neck" on the command line.  That 
gives you a flexible "neck" so that your facing 
isn't equal to your PBA's facing.  In this mode 
you can do things like look over your shoulder and 
shoot at the enemies behind you as you run away 
from them.  In this mode the [F] and keypad center 
keys turn your body to match your facing.  

Terra Nova does not support stereoscopic vision, 
only headtracking.  But it's cool anyway.





JOYSTICKS
---------

For joysticks, you'll need to edit your config 
file (tn.cfg in your Terra Nova directory.)

The tn.cfg will need a line reading "joystick ##"  
where ## is a number:

   0    standard joystick
   1    Flightstick Pro
   2    Thrustmaster
   3    none
  

Then, run Terra Nova and your joystick should work.

While playing:
   button 1     fires at your mouse cursor's 
                       current location.
   button 2     activates the jump jets.
   [Alt]-[J]    will recenter the joystick.

If you have a stick hat, the hat controls your 
head's viewing angle.  That is, pushing the hat up 
looks up, pushing it left looks left, and so on.  
Joystick button 3 (if you have one) recenters your 
head.
