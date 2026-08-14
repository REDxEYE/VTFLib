/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#pragma once

#include "VTFLibShared.h"
#include "VMTNode.h"


namespace VTFLib::Nodes {
    class VTFLIB_API CVMTValueNode : public CVMTNode {
    public:
        explicit CVMTValueNode(const char *name) : CVMTNode(name) {
        }

        ~CVMTValueNode() override = default;

        virtual void SetValue(const char *value) = 0;
    };
}
