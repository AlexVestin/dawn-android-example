## Dawn on Android with Vulkan backend
Basically just a quick of Dawn/Vulkan on android.

https://developer.android.com/ndk/guides/graphics/getting-started


`git clone https://dawn.googlesource.com/dawn`

Create new empty AndriodStudio project
Copy files from android_studio_files
* In build.gradle, change if needed:
  - file path to CMakeLists.txt
  - ndkVersion
  - applicationId
- In AndroidManifest.xml change the name of the package
