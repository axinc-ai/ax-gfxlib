#!/bin/sh

LIBDIR="lib.debug lib.release"
for libdir in $LIBDIR; do
  rm -rf $libdir
  mkdir $libdir
  if [ $? != 0 ]; then
    exit 1
  fi

  SDKDIR="ios ios_sim"
  for sdkdir in $SDKDIR; do
    mkdir $libdir/$sdkdir
    if [ $? != 0 ]; then
      exit 1
    fi
  done
done

cd ../build/xcode
xcodebuild -project axgl.xcodeproj clean

CONFIG="Debug Release"
for config in $CONFIG; do
  xcodebuild -project axgl.xcodeproj -configuration ${config} -sdk iphoneos -arch arm64
  if [ $? != 0 ]; then
    exit 2
  fi
  xcodebuild -project axgl.xcodeproj -configuration ${config} -sdk iphonesimulator -arch x86_64 -arch arm64
  if [ $? != 0 ]; then
    exit 2
  fi
done

cp build/Debug-iphoneos/libaxgl.a ../../lib.iOS/lib.debug/ios/
cp build/Debug-iphonesimulator/libaxgl.a ../../lib.iOS/lib.debug/ios_sim/
cp build/Release-iphoneos/libaxgl.a ../../lib.iOS/lib.release/ios/
cp build/Release-iphonesimulator/libaxgl.a ../../lib.iOS/lib.release/ios_sim/

