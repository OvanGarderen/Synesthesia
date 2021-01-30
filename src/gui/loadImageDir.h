#include <nanogui/screen.h>

#if defined(_WIN32)
#  include <windows.h>
#endif

#include <nanogui/opengl.h>
#include <map>
#include <thread>
#include <chrono>
#include <iostream>

#if !defined(_WIN32)
#  include <locale.h>
#  include <signal.h>
#  include <sys/dir.h>
#endif

std::vector<std::pair<int, std::string>>
loadImageDir(NVGcontext *ctx, const std::string &path, const char * extension=".png") {
    std::vector<std::pair<int, std::string> > result;
#if !defined(_WIN32)
    DIR *dp = opendir(path.c_str());
    if (!dp)
        throw std::runtime_error("Could not open image directory!");
    struct dirent *ep;
    while ((ep = readdir(dp))) {
        const char *fname = ep->d_name;
#else
    WIN32_FIND_DATA ffd;
    std::string searchPath = path + "/*.*";
    HANDLE handle = FindFirstFileA(searchPath.c_str(), &ffd);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Could not open image directory!");
    do {
        const char *fname = ffd.cFileName;
#endif
	size_t len = strlen(fname);
        if (len < strlen(extension) || strcmp(fname + len - strlen(extension), extension))
            continue;
        std::string fullName = path + "/" + std::string(fname);
        int img = nvgCreateImage(ctx, fullName.c_str(), 0);
        if (img == 0)
            throw std::runtime_error("Could not open image data!");
        result.push_back(
            std::make_pair(img, fullName.substr(0, fullName.length() - strlen(extension))));
#if !defined(_WIN32)
    }
    closedir(dp);
#else
    } while (FindNextFileA(handle, &ffd) != 0);
    FindClose(handle);
#endif
    return result;
}
