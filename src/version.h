#pragma once

#include "generated_git_version.h"

#define xstr(s) str(s)
#define str(s) #s

#define GIT_VERSION_STR \
    xstr(GIT_VERSION_MAJOR) "." \
    xstr(GIT_VERSION_MINOR) "." \
    xstr(GIT_VERSION_PATCH) "." \
    xstr(GIT_VERSION_REV) \
         GIT_VERSION_DIRTY

