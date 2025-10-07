@echo off

for %%f in (*.ui) do (
    echo Processing file: %%~nf.ui to %%~nf.py
    .venv\Scripts\pyuic6 "%%f" -o "%%~nf.py"
)
echo All .ui files have been processed.
pause