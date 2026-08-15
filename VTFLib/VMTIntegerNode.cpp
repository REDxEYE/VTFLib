/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VMTIntegerNode.h"

#include <cstdlib>

using namespace VTFLib::Nodes;

CVMTIntegerNode::CVMTIntegerNode(const char *name) : CVMTValueNode(name) {
    mValue = 0;
}

CVMTIntegerNode::CVMTIntegerNode(const char *name, const char *value) : CVMTValueNode(name) {
    CVMTIntegerNode::SetValue(value);
}

CVMTIntegerNode::CVMTIntegerNode(const char *name, int32_t value) : CVMTValueNode(name) {
    mValue = value;
}

CVMTIntegerNode::CVMTIntegerNode(const CVMTIntegerNode &other) : CVMTValueNode(other.GetName()) {
    mValue = other.mValue;
}

void CVMTIntegerNode::SetValue(const char *value) {
    mValue = static_cast<int32_t>(std::strtol(value, nullptr, 10));
}

void CVMTIntegerNode::SetValue(const int32_t value) {
    mValue = value;
}

int32_t CVMTIntegerNode::GetValue() const {
    return mValue;
}

VMTNodeType CVMTIntegerNode::GetType() const {
    return NODE_TYPE_INTEGER;
}

CVMTNode *CVMTIntegerNode::Clone() const {
    return new CVMTIntegerNode(*this);
}
