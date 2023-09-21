@echo off
@cls

rem Setup default locations.
@set HOME_DIR=d:\data\bas_svp
rem @set RAW_DATA_DIR="d:\data\raw data"
@set RAW_DATA_DIR="d:\data"
@set NET_DATA_DIR=u:\data

@set cruise=%1
@set cast=%2

rem get parameters if not supplied on command line
if "%cruise%" NEQ "" goto skip1

echo>$inp$.bas bad$="<|>=,;":on error goto done
echo>>$inp$.bas ? "Please enter cruise name: ";
echo>>$inp$.bas line input a$:if a$="" goto wrbat
echo>>$inp$.bas for i=1 to len(bad$)
echo>>$inp$.bas if not instr(a$,mid$(bad$,i,1))=0 then a$="(error)"
echo>>$inp$.bas next i
echo>>$inp$.bas wrbat:open "$inp$.bat" for output as #1
echo>>$inp$.bas ? #1,"set cruise=";a$
echo>>$inp$.bas done:close #1:system
qbasic /run $inp$.bas
call $inp$.bat
del $inp$.ba?

:skip1
if "%cast%" NEQ "" goto skip2

echo>$inp$.bas bad$="<|>=,;":on error goto done
echo>>$inp$.bas ? "Please enter cast name: ";
echo>>$inp$.bas line input a$:if a$="" goto wrbat
echo>>$inp$.bas for i=1 to len(bad$)
echo>>$inp$.bas if not instr(a$,mid$(bad$,i,1))=0 then a$="(error)"
echo>>$inp$.bas next i
echo>>$inp$.bas wrbat:open "$inp$.bat" for output as #1
echo>>$inp$.bas ? #1,"set cast=";a$
echo>>$inp$.bas done:close #1:system
qbasic /run $inp$.bas
call $inp$.bat
del $inp$.ba?

:skip2

rem Check for files
@echo Checking for existing directories
if not exist "%RAW_DATA_DIR%\%cruise%\%cruise%_%cast%.hex" goto error1
if not exist "%RAW_DATA_DIR%\%cruise%\%cruise%_%cast%.xmlcon" goto error2
if not exist "%NET_DATA_DIR%" mkdir "%NET_DATA_DIR%"
if not exist "%NET_DATA_DIR%\%cruise%" mkdir "%NET_DATA_DIR%\%cruise%"

rem transfer .bl .xmlcon .hdr .hex to net and unix dirs
rem transfer all files to network - JPRO 02/01/2008
@echo Backing up cast to network
copy "%RAW_DATA_DIR%\%cruise%\%cruise%_%cast%*.*"  "%NET_DATA_DIR%\%cruise%" >nul

if exist "%HOME_DIR%\%cruise%_%cast%_sbe35.asc" copy "%HOME_DIR%\%cruise%_%cast%_sbe35.asc" "%NET_DATA_DIR%\%curise%\%cruise%_%cast%_sbe35.asc"

rem transfer .hex and .xmlcon files to the working directory
@echo Preparing system
copy "%RAW_DATA_DIR%\%cruise%\%cruise%_%cast%.hex" "%HOME_DIR%\" >nul
copy "%RAW_DATA_DIR%\%cruise%\%cruise%_%cast%.xmlcon" "%HOME_DIR%\" >nul

rem call the SeaBird batch processing system to create the sound velocity profile
@echo Processing data
call SBEBatch D:/data/bas_svp/v_ctd_svp.txt %cruise%_%cast%
if not exist "%HOME_DIR%\%cruise%_%cast%svp.asc" goto error3

rem @echo Copy data to network:
copy "%HOME_DIR%\%cruise%_%cast%svp.asc" "%NET_DATA_DIR%\"

rem clean up the working directory
@echo Clean up
if not exist "%NET_DATA_DIR%\%cruise%_%cast%svp.asc" goto error4
del "%HOME_DIR%\%cruise%_%cast%.xmlcon"
del "%HOME_DIR%\%cruise%_%cast%.hex"
del "%HOME_DIR%\%cruise%_%cast%svp.cnv"
del "%HOME_DIR%\%cruise%_%cast%svp.asc"
goto end

:error1
echo "ERROR: %RAW_DATA_DIR%\%cruise%\%cruise%_%cast%.hex" does not exist
goto end

:error2
echo "ERROR: %RAW_DATA_DIR%\%cruise%\%cruise%_%cast%.xmlcon" does not exist
goto end


:error3
echo "ERROR: Process did not create profile %NET_DATA_DIR%\%cruise%_%cast%svp.asc"
goto end

:error4
echo "%HOME_DIR%\%curise%_%cast%svp.asc not copied to %NET_DATA_DIR%"
goto end

rem quit the batch process
:end
pause

