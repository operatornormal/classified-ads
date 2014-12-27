echo this needs to be run from qt command prompt
rd /q /s deps
mkdir deps
windeployqt --release --libdir=deps ..\release\classified-ads.exe
