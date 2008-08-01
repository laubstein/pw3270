# define the name of the installer
outfile "g3270_installer.exe"
 
# define the directory to install to
installDir $PROGRAMFILES\g3270
 
# default section
section
 
# define the output path for this file
setOutPath $INSTDIR
 
# define what to install and place it in the output path
file g3270.exe
file lib3270.dll
file fonts.conf
file g3270.conf
file w3n46.dll
file ui.xml
file g3270.jpg

# define uninstaller name
writeUninstaller $INSTDIR\uninstall.exe
 
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270" \
                 "DisplayName" "g3270 - 3270 emulator for windows/gtk"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
sectionEnd

# create a section to define what the uninstaller does.
# the section will always be named "Uninstall"
section "Uninstall"
 
# Always delete uninstaller first
delete $INSTDIR\uninstaller.exe
  
# now delete installed file
delete $INSTDIR\g3270.exe
delete $INSTDIR\lib3270.dll
delete $INSTDIR\fonts.conf
delete $INSTDIR\g3270.conf
delete $INSTDIR\w3n46.dll
delete $INSTDIR\ui.xml
delete $INSTDIR\g3270.jpg

# Remove registry 
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\g3270"

sectionEnd
