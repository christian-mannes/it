Here are the complete steps to notarize a Qt app for macOS to avoid security warnings:

## Prerequisites

### 1. Apple Developer Account
- Paid Apple Developer Program membership ($99/year)
- App-specific password for notarization

### 2. Developer Certificate
```bash
# List available certificates
security find-identity -v -p codesigning

This says:
1) AB86587400A7E3AAB19592427BC88D7BD41C8C52 "Developer ID Application: Christian Mannes (6CL5P9A99V)"
   1 valid identities found

# You need either:
# - "Developer ID Application: Your Name (TEAM_ID)" for distribution outside App Store
# - "Apple Development: Your Name (TEAM_ID)" for development/testing
```

### 3. App-Specific Password
1. Sign in to [appleid.apple.com](https://appleid.apple.com)
2. Go to Security → App-Specific Passwords
3. Generate password for "Xcode notarization"
4. Save this password securely

>> **The password for It3 is cuun-rvvf-rfrz-lzyw**

## Step 1: Configure Build for Signing

### CMakeLists.txt with Signing
```cmake
cmake_minimum_required(VERSION 3.21)
project(MyApp VERSION 1.0.0)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
    
    # Code signing configuration
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.MyApp")
    set(CODE_SIGN_IDENTITY "Developer ID Application: Your Name (TEAM_ID)")
    set(DEVELOPMENT_TEAM "YOUR_TEAM_ID")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

qt_add_executable(MyApp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

if(APPLE)
    set_target_properties(MyApp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "My Application"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_IDENTIFIER ${MACOSX_BUNDLE_GUI_IDENTIFIER}
        MACOSX_BUNDLE_ICON_FILE "MyApp.icns"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODE_SIGN_IDENTITY}
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM}
        XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS YES
    )
    
    # Add icon
    target_sources(MyApp PRIVATE MyApp.icns)
    set_source_files_properties(MyApp.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endif()

target_link_libraries(MyApp Qt6::Core Qt6::Widgets)
```

## Step 2: Create Entitlements File

### MyApp.entitlements
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.app-sandbox</key>
    <false/>
    <key>com.apple.security.cs.allow-jit</key>
    <true/>
    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
    <true/>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
    <key>com.apple.security.network.client</key>
    <true/>
    <key>com.apple.security.network.server</key>
    <true/>
    <!-- Add other entitlements as needed -->
</dict>
</plist>
```

## Step 3: Build and Deploy Qt Frameworks

### Build Script (build_and_sign.sh)
```bash
#!/bin/bash

APP_NAME="MyApp"
BUNDLE_ID="com.yourcompany.MyApp"
SIGNING_IDENTITY="Developer ID Application: Your Name (TEAM_ID)"
APPLE_ID="your.email@example.com"
TEAM_ID="YOUR_TEAM_ID"
APP_PASSWORD="your-app-specific-password"

echo "Building application..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

echo "Deploying Qt frameworks..."
macdeployqt ${APP_NAME}.app -verbose=2

echo "Signing all binaries and frameworks..."
# Sign all Qt frameworks first
find ${APP_NAME}.app/Contents/Frameworks -name "*.framework" -exec \
    codesign --force --verify --verbose --timestamp \
    --options runtime \
    --entitlements ../MyApp.entitlements \
    --sign "${SIGNING_IDENTITY}" {} \;

# Sign any plugins
find ${APP_NAME}.app/Contents/PlugIns -name "*.dylib" -exec \
    codesign --force --verify --verbose --timestamp \
    --options runtime \
    --entitlements ../MyApp.entitlements \
    --sign "${SIGNING_IDENTITY}" {} \;

# Sign the main executable
codesign --force --verify --verbose --timestamp \
    --options runtime \
    --entitlements ../MyApp.entitlements \
    --sign "${SIGNING_IDENTITY}" \
    ${APP_NAME}.app/Contents/MacOS/${APP_NAME}

# Sign the app bundle
codesign --force --verify --verbose --timestamp \
    --options runtime \
    --entitlements ../MyApp.entitlements \
    --sign "${SIGNING_IDENTITY}" \
    ${APP_NAME}.app

echo "Verifying code signature..."
codesign --verify --deep --strict --verbose=2 ${APP_NAME}.app
spctl -a -t exec -vv ${APP_NAME}.app

echo "Creating DMG..."
create-dmg \
    --volname "${APP_NAME} Installer" \
    --window-pos 200 120 \
    --window-size 600 400 \
    --icon-size 100 \
    --icon "${APP_NAME}.app" 175 120 \
    --hide-extension "${APP_NAME}.app" \
    --app-drop-link 425 120 \
    "${APP_NAME}-${VERSION}.dmg" \
    ${APP_NAME}.app

echo "Signing DMG..."
codesign --force --sign "${SIGNING_IDENTITY}" "${APP_NAME}-${VERSION}.dmg"

echo "Uploading for notarization..."
xcrun notarytool submit "${APP_NAME}-${VERSION}.dmg" \
    --apple-id "${APPLE_ID}" \
    --password "${APP_PASSWORD}" \
    --team-id "${TEAM_ID}" \
    --wait

echo "Stapling notarization..."
xcrun stapler staple "${APP_NAME}-${VERSION}.dmg"

echo "Verifying notarization..."
xcrun stapler validate "${APP_NAME}-${VERSION}.dmg"
spctl -a -t open --context context:primary-signature -v "${APP_NAME}-${VERSION}.dmg"

echo "Done! Your app is ready for distribution."
```

## Step 4: Manual Step-by-Step Process

If you prefer manual steps:

### 4.1 Build and Deploy
```bash
# Build
cmake --build build --config Release

# Deploy Qt frameworks
macdeployqt build/MyApp.app -verbose=2
```

### 4.2 Code Signing
```bash
# Sign frameworks
find build/MyApp.app/Contents/Frameworks -name "*.framework" -exec \
    codesign --force --verify --verbose --timestamp --options runtime \
    --sign "Developer ID Application: Your Name (TEAM_ID)" {} \;

# Sign plugins
find build/MyApp.app/Contents/PlugIns -name "*.dylib" -exec \
    codesign --force --verify --verbose --timestamp --options runtime \
    --sign "Developer ID Application: Your Name (TEAM_ID)" {} \;

# Sign main executable
codesign --force --verify --verbose --timestamp --options runtime \
    --entitlements MyApp.entitlements \
    --sign "Developer ID Application: Your Name (TEAM_ID)" \
    build/MyApp.app/Contents/MacOS/MyApp

# Sign app bundle
codesign --force --verify --verbose --timestamp --options runtime \
    --entitlements MyApp.entitlements \
    --sign "Developer ID Application: Your Name (TEAM_ID)" \
    build/MyApp.app
```

### 4.3 Create DMG
```bash
# Create DMG (you can use create-dmg tool or Disk Utility)
hdiutil create -volname "MyApp Installer" -srcfolder build/MyApp.app -ov -format UDZO MyApp-1.0.0.dmg

# Sign DMG
codesign --force --sign "Developer ID Application: Your Name (TEAM_ID)" MyApp-1.0.0.dmg
```

### 4.4 Notarize
```bash
# Submit for notarization
xcrun notarytool submit MyApp-1.0.0.dmg \
    --apple-id "your.email@example.com" \
    --password "your-app-specific-password" \
    --team-id "YOUR_TEAM_ID" \
    --wait

# Staple the notarization
xcrun stapler staple MyApp-1.0.0.dmg

# Verify
xcrun stapler validate MyApp-1.0.0.dmg
```

## Step 5: Verification

### Check Everything is Properly Signed and Notarized
```bash
# Verify code signature
codesign --verify --deep --strict --verbose=2 MyApp.app

# Check Gatekeeper acceptance
spctl -a -t exec -vv MyApp.app

# Verify notarization
xcrun stapler validate MyApp-1.0.0.dmg
```

## Common Issues and Solutions

### 1. Qt Framework Signing Issues
```bash
# If frameworks aren't signed properly, sign them individually:
codesign --force --sign "Developer ID Application: Your Name" \
    MyApp.app/Contents/Frameworks/QtCore.framework/Versions/A/QtCore
```

### 2. Entitlements Problems
- Remove sandbox entitlements if your app doesn't need them
- Add `com.apple.security.cs.allow-jit` for Qt applications
- Add network entitlements if your app uses networking

### 3. Notarization Failures
```bash
# Check notarization history and details
xcrun notarytool history --apple-id "your.email@example.com" \
    --password "your-app-specific-password" \
    --team-id "YOUR_TEAM_ID"

# Get detailed info about a submission
xcrun notarytool info SUBMISSION_ID \
    --apple-id "your.email@example.com" \
    --password "your-app-specific-password" \
    --team-id "YOUR_TEAM_ID"
```

## Final Notes

1. **Test thoroughly**: Test the signed/notarized app on a clean Mac
2. **Automate**: Use CI/CD for consistent builds
3. **Keep certificates current**: Developer ID certificates expire
4. **Monitor Apple changes**: Notarization requirements can change

Once completed, users will be able to download and run your Qt app without any security warnings from macOS Gatekeeper.
