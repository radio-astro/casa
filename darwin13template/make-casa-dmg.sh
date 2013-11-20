casaver=$(cut -d ' ' -f 2 CASA/CASA.app/Contents/Resources/VERSION)
hdiutil create -srcfolder CASA -volname "CASA Macintosh Intel ${casaver}" c1
hdiutil convert -format UDRW -o c2 c1.dmg && rm c1.dmg
#open c2.dmg
echo
echo "Fix the Finder window of the CASA disk image (icon size and position)"
echo "and then run this command:"
hdiutil convert -format UDBZ -o CASA-intel-${casaver} c2.dmg
#osascript -e 'tell application "Finder" to activate'
