Name "g3270"
Caption "g3270 - 3270 Emulator for windows/gtk"
outfile "g3270_installer.exe"
XPStyle on
 
# define the directory to install to
installDir $PROGRAMFILES\g3270
 
RequestExecutionLevel admin

# default section
section
 
# define the output path for this file
setOutPath $INSTDIR
 
createShortCut "$SMPROGRAMS\g3270.lnk" "$INSTDIR\g3270.exe"

# Binary files
file bin\Release\g3270.exe
file bin\Release\lib3270.dll
file bin\Release\w3n46.dll

# UI definition files
file src\g3270\fonts.conf
file src\g3270\ui.xml
file src\g3270\g3270.jpg
file src\g3270\g3270.ico

# Default configuration file
file g3270.conf

# Locale files
CreateDirectory "$INSTDIR\locale\pt_BR\LC_MESSAGES"
file "/oname=$INSTDIR\locale\pt_BR\LC_MESSAGES\g3270.mo" "locale\pt_BR\LC_MESSAGES\g3270.mo"

# define uninstaller name
writeUninstaller $INSTDIR\uninstall.exe
 
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270" \
                 "DisplayName" "g3270 - 3270 emulator for windows/gtk"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270" \
                 "InstallLocation" "$INSTDIR"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270" \
                 "NoModify" "1"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270" \
                 "NoRepair" "1"

sectionEnd

# create a section to define what the uninstaller does.
# the section will always be named "Uninstall"
section "Uninstall"
 
# Always delete uninstaller first
delete $INSTDIR\uninstaller.exe
  
# now delete installed files
delete $INSTDIR\g3270.exe
delete $INSTDIR\lib3270.dll
delete $INSTDIR\fonts.conf
delete $INSTDIR\g3270.conf
delete $INSTDIR\w3n46.dll
delete $INSTDIR\ui.xml
delete $INSTDIR\g3270.jpg
delete $INSTDIR\g3270.ico
delete $INSTDIR\g3270.log
delete "$SMPROGRAMS\g3270.lnk"
rmdir /r "$INSTDIR\locale"

# Remove registry 
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270"

sectionEnd
