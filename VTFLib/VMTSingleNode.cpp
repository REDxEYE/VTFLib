/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VMTSingleNode.h"

#include <cstdlib>

using namespace VTFLib::Nodes;

CVMTSingleNode::CVMTSingleNode(const char *name) : CVMTValueNode(name) {
    mValue = 0.0f;
}

CVMTSingleNode::CVMTSingleNode(const char *name, const char *value) : CVMTValueNode(name) {
    CVMTSingleNode::SetValue(value);
}

CVMTSingleNode::CVMTSingleNode(const char *name, const float value) : CVMTValueNode(name) {
    mValue = value;
}

CVMTSingleNode::CVMTSingleNode(const CVMTSingleNode &other) : CVMTValueNode(other.GetName()) {
    mValue = other.mValue;
}

void CVMTSingleNode::SetValue(const char *value) {
    mValue = std::strtof(value, nullptr);
}

void CVMTSingleNode::SetValue(float value) {
    mValue = value;
}

float CVMTSingleNode::GetValue() const {
    return mValue;
}

VMTNodeType CVMTSingleNode::GetType() const {
    return NODE_TYPE_SINGLE;
}

CVMTNode *CVMTSingleNode::Clone() const {
    return new CVMTSingleNode(*this);
}
