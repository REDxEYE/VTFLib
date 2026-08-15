//
// Created by red_eye on 8/15/26.
//

#pragma once

#include "compressonator.h"

CMP_ERROR CreateMipSet(
    CMP_MipSet &mipSet,
    const void *data,
    int width,
    int height,
    CMP_FORMAT format);
