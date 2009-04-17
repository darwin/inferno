REM this file recompiles all *.cg to *.cg.shd in current directory
REM current constants.h and defines.h are included in .cg files
REM
REM note: check your cgc instalation, if cgc.exe is on system path
REM
for %%i in (*_vp.cg) do call cgc.exe -profile vp30 -o %%i.shd %%i
for %%i in (*_fp.cg) do call cgc.exe -profile fp30 -o %%i.shd %%i
