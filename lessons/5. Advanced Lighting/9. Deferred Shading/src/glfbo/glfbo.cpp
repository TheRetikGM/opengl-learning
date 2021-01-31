#include "glfbo.h"
#include <stb_image_write.h>

GLint glfbo::GetFramebufferBinding()
{
    GLint bind = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bind);
    return bind;
}
std::string glfbo::GetScreenshotName(glfbo::ImageType type)
{
    time_t tim = time(NULL);
    struct tm *ltm = localtime(&tim);
    
    char buf[50];
    snprintf(buf, sizeof(buf), ("screenshot_%i%02i%02i_%02i%02i%02i." + ImageTypeStrings[(size_t)type]).c_str(),
    ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return std::string(buf);
}