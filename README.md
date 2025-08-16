# eqclientmod for EverQuest EQMac client
solar@heliacal.net

This is a game mod implemented as a planted DLL file.  It works by wrapping an existing DLL the game already uses and because it's in the game directory it's loaded before the one in the system directory.

I use Visual Studio 2022 for this but you can adapt it to work with other versions and possibly even make it work on Win9x by using VS2005 or older.  I only use the Release configuration to build.
By default the mod wraps winmm.dll.  It is possible to switch the 'personality' of the mod to use dinput8.dll or winmm.dll, check personality.h.

I'm providing these because I find them useful and other developers may learn from and extend this for their own projects.
Note that these mods can alter gameplay and may be considered cheating by some people.  Please respect the wishes of emu server operators and other players, and don't use these mods to break rules or ruin the experience of others.

# Installation
To install just copy `winmm.dll` to the game directory (the directory that contains `eqgame.exe`).
To uninstall remove or rename `winmm.dll`.

Once the game runs the mod will create a config file named `eqclientmod.ini` which contains toggles for the included mods along with a brief description of what they do.

You can test that it's working by typing `/eqclientmod` in game which should give you a version string if the mod loaded properly.

# Mods


### Disable Gamma Change
The game has a gamma slider in it but it does it in a really annoying way that changes the desktop gamma.  It tries to restore it when the game exits but this doesn't always happen with multiple clients and crashes.
This mod disables the gamma functionality by detouring the function that would do this and instead doing nothing.  If you like the gamma, make sure you edit `eqclientmod.ini` and disable this mod.


### CommandHandler
This mod adds the extra command handling that some of the other hacks use but it's not strictly necessary to enable this to use the other hacks.  Basic commands included: /eqclientmod /crash


### Player Window Auto Attack Indicator
This makes the auto attack status more visible in the border of the player window (the one with your name and health/mana).
The rectangle around the inside of the window is called a lasso here, you can use LassoThickness to adjust how thick the rectangle is.
Set ColorFunction to 1 for a red/white pulsing effect or 2 for a rainbow effect.  Set ColorSpeed to control how fast the colors cycle.


### Field of View
Normally EQ expects to render to a 4:3 display and anything wider results in a zoomed in telescope effect (same horizontal field of vision, smaller vertical).
This mod changes the calculation so the horizontal field of view can expand at wider ratios.  Note that FoV is not something that should be freely adjusted for an
authentic experience - it's tied to game effects like magnification spells and intoxication (which reduce the field of view) as well as some buffs that increase the field of view.
You can forcefully set the FoV with the /fov command but be aware that it will change again when one of these effects happens.


### Autofire
This adds an /autofire toggle command that keeps spamming range attack.  It is simplistic and doesn't interact with targets or regular auto attack so you need to take care to turn auto attack on and off yourself when switching to/from ranged attacks.
Autofire will turn itself off if there is no target when it next tried to attack, but it is not totally safe and you can set a new target and accidentally shoot it.  This is a useful feature of it for me but may not be for everybody.


### Character Select Rotation
This hack makes the character selection screen rotation smooth.  If you are using some kind of FPS limiter I recommend not doing that, because this looks way better (to me).


### Autofollow
This mod improves /follow reliability.  There is logic in /follow to turn run mode on and off and this actually makes your character crash out of the game if your framerate is high enough.
There is also a smooth turning function to circle around to the followed target which is framerate dependent and causes follow failures.  Both of these things are disabled by this mod.


### Hidecorpse Looted
This adds /hidecorpses LOOTED like what is available in newer clients.


### Farclip
This increases the draw distance so you can see farther ahead.  This has a performance impact and is really bad in some zones.  
You can use the /farclip command to temporarily change the distance but it's set automatically each time a zone is loaded.
ExtendFog moves the fog out to the edge of the normal clip plane when underwater and during rain/snow.  This helps visibility and makes the game look better but performance can be bad in some zones.
The command /uval will toggle a feature called UseVisActorList.  Normally the game doesn't update actors (mobs) that are far away or out of view, so they appear to drop from the sky once you get close enough and they finally get updated positions.
This toggle will make it so all actors are always updated which looks nicer especially with extended draw distance but it has a performance cost in zones with many actors.  There is also a limit to how far the server sends movement updates so the benefit of this is limited.


### Trig functions
This hack improves the quality of the trigonometry functions used in some places, smoothing out mouse control.
This isn't my own work, I copied it from the dll used by Project Quarm.


### 4K friendly double sized UI
To use this you need to copy uifiles/defaultx2 to your uifiles directory.
Also check uifiles/defaultx2-customize/ui-notes.txt
This doubles the size of the UI, including fonts, which makes the game legible on a 4K display.  There are some nuances that need to be understood to use this successfully.  If you have a 4K display chances are that it's a high DPI and windows
is using scaling for everything to make it legible.  There is a slider in the settings, probably set to something like 125%, 150% or similar.  What this does with old applications like EQ is it simply stretches the window, while the program continues
to think it's rendering to a smaller one.  This can make the UI elements look larger even without this mod, but you should make sure this is not happening before using this mod.
Right click eqgame.exe, properties, compatibility tab, change DPI settings, check the box "Override high DPI scaling behavior" and select "Application" in the dropdown.
You most likely have to do this even to modern live everquest if you're using a high DPI screen and you want to avoid the stretching.
Once you do this, if you want the full screen 4K experience, turn on this mod in the ini file and /load defaultx2 in game.  Set your eqclient.ini with Width=3840 Height=2160 and WindowedMode=FALSE (borderless fullscreen).
You can use this at 2560x1600 too but it might be a little crowded.  This is pretty subjective and depends how far away you sit from your TV/monitor.  I use it at both resolutions and it's fine for me because I like the UI elements to be large.
It is not possible to have a 1.5 or other fractional scale, just 2x, 3x etc.
Currently there is an issue with the starting city and deity selection windows during character creation not being large enough.


### Nameplate
Changes nameplate behavior to make them always visible, not hidden with distance.  This is helpful together with the farclip hack.  There is also a scaling option which is useful with the defaultx2 4K UI.


### Buff window tooltips
This hack adds remaining time and counters to the tooltips in the buff window.


### Debug
This hack writes dbg-XXX.txt files with the process id in the filename.  It is not useful for general play but I use this during development and to diagnose client crashes.
