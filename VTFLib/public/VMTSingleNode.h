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
#include "VMTValueNode.h"


namespace VTFLib::Nodes {
    class VTFLIB_API CVMTSingleNode : public CVMTValueNode {
    public:
        CVMTSingleNode(const char *name);

        CVMTSingleNode(const char *name, const char *value);

        CVMTSingleNode(const char *name, float value);

        CVMTSingleNode(const CVMTSingleNode &other);

        ~CVMTSingleNode() override = default;

        void SetValue(const char *value) override;

        void SetValue(float value);

        [[nodiscard]] float GetValue() const;

        [[nodiscard]] VMTNodeType GetType() const override;

        [[nodiscard]] CVMTNode *Clone() const override;

    private:
        float mValue{};
    };
}
