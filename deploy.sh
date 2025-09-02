#!/bin/bash

DIR=~/Code/it3/build/Qt_6_9_2_for_macOS-Release
PROJ_DIR=~/Code/it3
APP_NAME="It"
BUNDLE_ID="com.mannestech.it3"
SIGNING_IDENTITY="Developer ID Application: Christian Mannes (6CL5P9A99V)"
APPLE_ID="christ@mannes-tech.com"
TEAM_ID="6CL5P9A99V"
APP_PASSWORD="cuun-rvvf-rfrz-lzyw"
MACDEPLOY=/Users/christ/Qt/6.9.2/macos/bin/macdeployqt6

cd "$PROJ_DIR"
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
        --sign "${SIGNING_IDENTITY}" "$framework"
done

find It.app/Contents/PlugIns -name "*.dylib" -exec codesign \
    --force --sign "${SIGNING_IDENTITY}" {} \;

codesign --force --verify --verbose --timestamp --options runtime \
    --entitlements "${PROJ_DIR}/It.entitlements" \
    --sign "${SIGNING_IDENTITY}" It.app

codesign --verify --deep --strict It.app

# Create a zip or dmg of your app
ditto -c -k --keepParent It.app It.zip

# Maybe you need this...(first time only)
# xcrun notarytool store-credentials

# Submit for notarization
xcrun notarytool submit It.zip --keychain-profile "notarytool-password" --wait
# If something goes wrong, paste in the UUID from the previous step and:
#xcrun notarytool log 959c986d-38e6-414d-840a-2c2c012ff740 --keychain-profile "notarytool-password"

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

echo "DONE!"
