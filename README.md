## Rendering example of Dawn on Android with Vulkan backend


### Preparations
https://developer.android.com/ndk/guides/graphics/getting-started

### Get slightly modified dawn
```
git clone https://github.com/AlexVestin/dawn-android
```

### Import into andriod studio
``` 
Create new empty AndriodStudio project
Copy files from android_studio_files
- In build.gradle, change if needed:
  - file path to CMakeLists.txt
  - ndkVersion
- In AndroidManifest.xml change the name of the package
```

Should be good to build after that


https://dawn.googlesource.com/dawn