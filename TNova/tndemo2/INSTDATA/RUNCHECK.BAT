@echo off
if exist c:\_tnchkcd.bat goto start_chk
start %1
goto done
:start_chk
start %2
:done
