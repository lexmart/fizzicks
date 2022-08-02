@echo off

del *.pdb > nul 2> nul

echo asdfasdfasdf > lock.tmp
cl /Zi /LD fizzicks.cpp /link /NOLOGO /export:tick /incremental:no -PDB:fizzicks_%random%.pdb
del lock.tmp

cl /Zi fizzicks_win32.cpp /I .\include\SDL2 /link .\lib\x64\SDL2.lib .\lib\x64\SDL2main.lib /SUBSYSTEM:WINDOWS /NODEFAULTLIB:libcmtd.lib opengl32.lib /NOLOGO /incremental:no
