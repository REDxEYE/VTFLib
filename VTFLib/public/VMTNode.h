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

typedef enum tagVMTNodeType {
    NODE_TYPE_GROUP = 0,
    NODE_TYPE_GROUP_END,
    NODE_TYPE_STRING,
    NODE_TYPE_INTEGER,
    NODE_TYPE_SINGLE,
    NODE_TYPE_COUNT
} VMTNodeType;

namespace VTFLib::Nodes {
    class CVMTGroupNode;

    class VTFLIB_API CVMTNode {
    public:
        explicit CVMTNode(const char *name);

        virtual ~CVMTNode();

        [[nodiscard]] const char *GetName() const;

        void SetName(const char *name);

        [[nodiscard]] CVMTGroupNode *GetParent() const;

        [[nodiscard]] virtual VMTNodeType GetType() const = 0;

        [[nodiscard]] virtual CVMTNode *Clone() const = 0;

    private:
        friend class CVMTGroupNode; // For direct parent setting.
        char *mName;
        CVMTGroupNode *mParent;
    };
}
