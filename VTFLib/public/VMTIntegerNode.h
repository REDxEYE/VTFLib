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
    class VTFLIB_API CVMTIntegerNode : public CVMTValueNode {
    public:
        explicit CVMTIntegerNode(const char *name);

        CVMTIntegerNode(const char *name, const char *value);

        CVMTIntegerNode(const char *name, int32_t value);

        CVMTIntegerNode(const CVMTIntegerNode &other);

        ~CVMTIntegerNode() override = default;

        void SetValue(const char *value) override;

        void SetValue(int32_t value);

        [[nodiscard]] int32_t GetValue() const;

        [[nodiscard]] VMTNodeType GetType() const override;

        [[nodiscard]] CVMTNode *Clone() const override;

    private:
        int32_t mValue{};
    };
}
