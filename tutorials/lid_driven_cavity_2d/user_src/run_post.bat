@echo off
:: run_post.bat
:: Argument %1 = image prefix  e.g. rCFD_mix_topright_
:: This bat file is copied INTO the post folder by rCFD_r9.
:: It uses its own location (%~dp0) as working dir -- no path doubling.

set PYTHONPATH=
set PYTHONHOME=
set PYTHONSTARTUP=

set PYTHON=C:\Users\zahid\rcfd_env\Scripts\python.exe
set IMG_PREFIX=%1

:: cd into the folder where this bat file lives (= post folder)
cd /d "%~dp0"

echo ============================================================
echo  rCFD post-processing
echo  Folder : %CD%
echo  Prefix : %IMG_PREFIX%
echo ============================================================

echo [1/3] Plotting Centre of Gravity ...
"%PYTHON%" "plot_CoG.py"

echo [2/3] Plotting Mixing Index ...
"%PYTHON%" "plot_mix_ind.py"

echo [3/3] Building MP4 animation ...
"%PYTHON%" "make_animation.py" %IMG_PREFIX%

echo ============================================================
echo  Done. Check above folder for PNG and MP4 files.
echo ============================================================
pause
