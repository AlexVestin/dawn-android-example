/*
 * Vulkan Samples
 *
 * Copyright (C) 2015-2016 Valve Corporation
 * Copyright (C) 2015-2016 LunarG, Inc.
 * Copyright (C) 2015-2016 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
VULKAN_SAMPLE_DESCRIPTION
samples utility functions
*/

#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>

#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>

#include "util.h"

// Android specific include files.
#include <unordered_map>

// Header files.
#include "string.h"
#include "errno.h"
#include <native_app_glue/android_native_app_glue.h>
// Static variable that keeps ANativeWindow and asset manager instances.
static android_app *Android_application = nullptr;

std::string get_file_name(const std::string &s) {
    char sep = '/';

#ifdef _WIN32
    sep = '\\';
#endif

    // cout << "in get_file_name\n";
    size_t i = s.rfind(sep, s.length());
    if (i != std::string::npos) {
        return (s.substr(i + 1, s.length() - i));
    }

    return ("");
}


std::string get_data_dir(std::string filename) {
    std::string basedir = "";
    // get the base filename
    std::string fnames = get_file_name(filename);

    // get the prefix of the base filename, i.e. the part before the dash
    std::stringstream sstream(fnames);
    std::string prefix;
    std::getline(sstream, prefix, '-');
    std::string ddir = basedir + prefix;
    return ddir;
}




bool read_ppm(char const *const filename, int &width, int &height, uint64_t rowPitch, unsigned char *dataPtr) {
    // PPM format expected from http://netpbm.sourceforge.net/doc/ppm.html
    //  1. magic number
    //  2. whitespace
    //  3. width
    //  4. whitespace
    //  5. height
    //  6. whitespace
    //  7. max color value
    //  8. whitespace
    //  7. data

    // Comments are not supported, but are detected and we kick out
    // Only 8 bits per channel is supported
    // If dataPtr is nullptr, only width and height are returned

    // Read in values from the PPM file as characters to check for comments
    char magicStr[3] = {}, heightStr[6] = {}, widthStr[6] = {}, formatStr[6] = {};

#ifndef __ANDROID__
    FILE *fPtr = fopen(filename, "rb");
#else
    FILE *fPtr = AndroidFopen(filename, "rb");
#endif
    if (!fPtr) {
        printf("Bad filename in read_ppm: %s\n", filename);
        return false;
    }

    // Read the four values from file, accounting with any and all whitepace
    int U_ASSERT_ONLY count = fscanf(fPtr, "%s %s %s %s ", magicStr, widthStr, heightStr, formatStr);
    assert(count == 4);

    // Kick out if comments present
    if (magicStr[0] == '#' || widthStr[0] == '#' || heightStr[0] == '#' || formatStr[0] == '#') {
        printf("Unhandled comment in PPM file\n");
        return false;
    }

    // Only one magic value is valid
    if (strncmp(magicStr, "P6", sizeof(magicStr))) {
        printf("Unhandled PPM magic number: %s\n", magicStr);
        return false;
    }

    width = atoi(widthStr);
    height = atoi(heightStr);

    // Ensure we got something sane for width/height
    static const int saneDimension = 32768;  //??
    if (width <= 0 || width > saneDimension) {
        printf("Width seems wrong.  Update read_ppm if not: %u\n", width);
        return false;
    }
    if (height <= 0 || height > saneDimension) {
        printf("Height seems wrong.  Update read_ppm if not: %u\n", height);
        return false;
    }

    if (dataPtr == nullptr) {
        // If no destination pointer, caller only wanted dimensions
        return true;
    }

    // Now read the data
    for (int y = 0; y < height; y++) {
        unsigned char *rowPtr = dataPtr;
        for (int x = 0; x < width; x++) {
            count = fread(rowPtr, 3, 1, fPtr);
            assert(count == 1);
            rowPtr[3] = 255; /* Alpha of 1 */
            rowPtr += 4;
        }
        dataPtr += rowPitch;
    }
    fclose(fPtr);

    return true;
}

void wait_seconds(int seconds) {
#ifdef WIN32
    Sleep(seconds * 1000);
#elif defined(__ANDROID__)
    sleep(seconds);
#else
    //std::sleep(seconds);
#endif
}

timestamp_t get_milliseconds() {
#ifdef WIN32
    LARGE_INTEGER frequency;
    BOOL useQPC = QueryPerformanceFrequency(&frequency);
    if (useQPC) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / frequency.QuadPart;
    } else {
        return GetTickCount();
    }
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_usec / 1000) + (timestamp_t)now.tv_sec;
#endif
}


std::string get_file_directory() {
    assert(Android_application != nullptr);
    return Android_application->activity->externalDataPath;
}

//
// Android specific helper functions.
//

// Helpder class to forward the cout/cerr output to logcat derived from:
// http://stackoverflow.com/questions/8870174/is-stdcout-usable-in-android-ndk
class AndroidBuffer : public std::streambuf {
   public:
    AndroidBuffer(android_LogPriority priority) {
        priority_ = priority;
        this->setp(buffer_, buffer_ + kBufferSize - 1);
    }

   private:
    static const int32_t kBufferSize = 128;
    int32_t overflow(int32_t c) {
        if (c == traits_type::eof()) {
            *this->pptr() = traits_type::to_char_type(c);
            this->sbumpc();
        }
        return this->sync() ? traits_type::eof() : traits_type::not_eof(c);
    }

    int32_t sync() {
        int32_t rc = 0;
        if (this->pbase() != this->pptr()) {
            char writebuf[kBufferSize + 1];
            memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
            writebuf[this->pptr() - this->pbase()] = '\0';

            rc = __android_log_write(priority_, "std", writebuf) > 0;
            this->setp(buffer_, buffer_ + kBufferSize - 1);
        }
        return rc;
    }

    android_LogPriority priority_ = ANDROID_LOG_INFO;
    char buffer_[kBufferSize];
};



void Android_handle_cmd(android_app *app, int32_t cmd) {
    
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            // The window is being shown, get it ready.
            int32_t w   = ANativeWindow_getWidth(app->window);
            int32_t h   = ANativeWindow_getHeight(app->window);
            int32_t fmt = ANativeWindow_getFormat(app->window);

            sample_main(app->window, cmd, w, h, fmt);
            LOGI("\n");
            LOGI("=================================================");
            LOGI("          The sample ran successfully!!");
            LOGI("=================================================");
            LOGI("\n");
            break;
        }
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            break;
        default:
            LOGI("event not handled: %d", cmd);
    }
}

bool Android_process_command() {
    assert(Android_application != nullptr);
    int events;
    android_poll_source *source;
    // Poll all pending events.
    if (ALooper_pollAll(0, NULL, &events, (void **)&source) >= 0) {
        // Process each polled events
        if (source != NULL) source->process(Android_application, source);
    }
    return Android_application->destroyRequested;
}

void android_main(struct android_app *app) {
    // Set static variables.
    Android_application = app;
    // Set the callback to process system events
    app->onAppCmd = Android_handle_cmd;


    // Forward cout/cerr to logcat.
    std::cout.rdbuf(new AndroidBuffer(ANDROID_LOG_INFO));
    std::cerr.rdbuf(new AndroidBuffer(ANDROID_LOG_ERROR));

    // Main loop
    do {
        Android_process_command();
    }  // Check if system requested to quit the application
    while (app->destroyRequested == 0);

    return;
}

ANativeWindow *AndroidGetApplicationWindow() {
    assert(Android_application != nullptr);
    return Android_application->window;
}

bool AndroidFillShaderMap(const char *path, std::unordered_map<std::string, std::string> *map_shaders) {
    assert(Android_application != nullptr);
    auto directory = AAssetManager_openDir(Android_application->activity->assetManager, path);

    const char *name = nullptr;
    while (1) {
        name = AAssetDir_getNextFileName(directory);
        if (name == nullptr) {
            break;
        }

        std::string file_name = name;
        if (file_name.find(".frag") != std::string::npos || file_name.find(".vert") != std::string::npos) {
            // Add path to the filename.
            file_name = std::string(path) + "/" + file_name;
            std::string shader;
            if (!AndroidLoadFile(file_name.c_str(), &shader)) {
                continue;
            }
            // Remove \n to make the lookup more robust.
            while (1) {
                auto ret_pos = shader.find("\n");
                if (ret_pos == std::string::npos) {
                    break;
                }
                shader.erase(ret_pos, 1);
            }

            auto pos = file_name.find_last_of(".");
            if (pos == std::string::npos) {
                // Invalid file nmae.
                continue;
            }
            // Generate filename for SPIRV binary.
            std::string spirv_name = file_name.replace(pos, 1, "-") + ".spirv";
            // Store the SPIRV file name with GLSL contents as a key.
            // The file contents can be long, but as we are using unordered map, it wouldn't take
            // much storage space.
            // Put the file into the map.
            (*map_shaders)[shader] = spirv_name;
        }
    };

    AAssetDir_close(directory);
    return true;
}

bool AndroidLoadFile(const char *filePath, std::string *data) {
    assert(Android_application != nullptr);
    AAsset *file = AAssetManager_open(Android_application->activity->assetManager, filePath, AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(file);
    LOGI("Loaded file:%s size:%zu", filePath, fileLength);
    if (fileLength == 0) {
        return false;
    }
    data->resize(fileLength);
    AAsset_read(file, &(*data)[0], fileLength);
    return true;
}

void AndroidGetWindowSize(int32_t *width, int32_t *height) {
    // On Android, retrieve the window size from the native window.
    assert(Android_application != nullptr);
    *width = ANativeWindow_getWidth(Android_application->window);
    *height = ANativeWindow_getHeight(Android_application->window);
}

// Android fopen stub described at
// http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/#comment-1850768990
static int android_read(void *cookie, char *buf, int size) { return AAsset_read((AAsset *)cookie, buf, size); }

static int android_write(void *cookie, const char *buf, int size) {
    return EACCES;  // can't provide write access to the apk
}

static fpos_t android_seek(void *cookie, fpos_t offset, int whence) { return AAsset_seek((AAsset *)cookie, offset, whence); }

static int android_close(void *cookie) {
    AAsset_close((AAsset *)cookie);
    return 0;
}

FILE *AndroidFopen(const char *fname, const char *mode) {
    if (mode[0] == 'w') {
        return NULL;
    }

    assert(Android_application != nullptr);
    AAsset *asset = AAssetManager_open(Android_application->activity->assetManager, fname, 0);
    if (!asset) {
        return NULL;
    }

    return funopen(asset, android_read, android_write, android_seek, android_close);
}
