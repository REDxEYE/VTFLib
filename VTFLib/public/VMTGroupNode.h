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

#include "VMTStringNode.h"
#include "VMTIntegerNode.h"
#include "VMTSingleNode.h"

#include <list>

namespace VTFLib {
    class CVMTFile;

    namespace Nodes {
        class VTFLIB_API CVMTGroupNode : public CVMTNode {
        public:
            explicit CVMTGroupNode(const char *name);

            CVMTGroupNode(const CVMTGroupNode &other);

            ~CVMTGroupNode() override;

            [[nodiscard]] VMTNodeType GetType() const override;

            [[nodiscard]] CVMTNode *Clone() const override;

            [[nodiscard]] size_t GetNodeCount() const;

            CVMTNode *AddNode(CVMTNode *node);

            CVMTGroupNode *AddGroupNode(const char *name);

            CVMTStringNode *AddStringNode(const char *name, const char *value);

            CVMTIntegerNode *AddIntegerNode(const char *name, int32_t value);

            CVMTSingleNode *AddSingleNode(const char *name, float value);

            void RemoveNode(const CVMTNode *node) const;

            void RemoveAllNodes() const;

            [[nodiscard]] CVMTNode *GetNode(size_t index) const;

            CVMTNode *GetNode(const char *name) const;

        private:
            typedef std::list<CVMTNode *> CVMTNodeList;
            CVMTNodeList *mVMTNodeList;
        };
    }
}
