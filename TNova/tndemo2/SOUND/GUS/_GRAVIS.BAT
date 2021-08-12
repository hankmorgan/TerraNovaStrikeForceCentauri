@echo off
rem Gravis patch load attempt, as there is no way to do this from 
rem  inside the game code that i know about, sadly
loadpats -Iff.ini -Q
if errorlevel 1 goto patch_fail
echo UltraSound patches loaded successfully
goto end
:patch_fail
echo UltraSound patches failed to load, sorry
:end
