#include "util.h"
#include "lib.h"

// TODO: remove?


void sample_main(ANativeWindow* window, int32_t command, int w, int h, int fmt) {
    printf("We in here new test 11 %p %d %d %d\n", window, w, h, fmt);

    DawnAndroid::Init(window, w, h, fmt);
    printf("Initialised?? :D\n");
    DawnAndroid::Frame();
    printf("Frame?? :D\n");
}
