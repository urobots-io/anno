#pragma once
#include <string>
#include <sstream>

namespace git_info {
    extern const char *URL;
    extern const char *branch;
    extern const char *revision;
    extern const char *modified;
    extern const char *commit_time;
    extern const char *build_time;

    static std::string RepositoryVersion()
    {
        std::stringstream ss;
        ss << branch << ", " << revision << " (" << commit_time << "), " << git_info::modified;
        return ss.str();
    }
}
