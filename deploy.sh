#!/bin/bash
#
# Must run in main directory
# Build release before running.
#
# Must set (environment or shell): IT_SIGNING_IDENTITY
# IT_SIGNING_IDENTITY="Developer ID Application: Your Name (TEAM_ID)"
DIR=build/Qt_6_9_2_for_macOS-Release
APP_NAME="It"
MACDEPLOY=/Users/christ/Qt/6.9.2/macos/bin/macdeployqt6

rm -f it.zip
zip -r it.zip it
mv it.zip "$DIR/It.app/Contents/Resources"
cd "$DIR"

rm -rf It.zip
xattr -cr It.app

# Create qt.conf in the Resources directory
cat > It.app/Contents/Resources/qt.conf << EOF
[Paths]
Plugins = PlugIns
EOF

$MACDEPLOY It.app -always-overwrite -appstore-compliant

xattr -cr It.app

codesign --remove-signature It.app/Contents/MacOS/It 2>/dev/null || true
codesign --remove-signature It.app 2>/dev/null || true

find It.app -name "*.framework" -type d | while read framework; do
    codesign --force --verify --verbose --timestamp --options runtime \
        --sign "${IT_SIGNING_IDENTITY}" "$framework"
done

find It.app/Contents/PlugIns -name "*.dylib" -exec codesign \
    --force --sign "${IT_SIGNING_IDENTITY}" {} \;

codesign --force --verify --verbose --timestamp --options runtime \
    --entitlements "../../It.entitlements" \
    --sign "${IT_SIGNING_IDENTITY}" It.app

codesign --verify --deep --strict It.app

# Create a zip or dmg of your app
ditto -c -k --keepParent It.app It.zip

# Maybe you need this...(first time only)
# xcrun notarytool store-credentials

# Submit for notarization
xcrun notarytool submit It.zip --keychain-profile "notarytool-password" --wait
# If something goes wrong, paste in the UUID from the previous step and:
#xcrun notarytool log <uuid> --keychain-profile "notarytool-password"

# Staple the notarization ticket
xcrun stapler staple It.app
# Verify it worked - gatekeeper test (optional)
#spctl -a -t exec -vv It.app

# Package in a DMG
mkdir dmg_contents
cp -R It.app dmg_contents/

# 2. Create Applications shortcut
ln -s /Applications dmg_contents/Applications

# 3. Create the DMG
hdiutil create -volname "It" -srcfolder dmg_contents -ov -format UDZO It.dmg

# 4. Clean up
rm -rf dmg_contents

# 5. Copy dmg to download directory (doc)
mv It.dmg ../../doc

echo "DONE!"
