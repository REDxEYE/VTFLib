/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VMTNode.h"

#include <cstring>

using namespace VTFLib::Nodes;

CVMTNode::CVMTNode(const char *name) {
    mName = new char[strlen(name) + 1];
    strcpy(mName, name);
    mParent = nullptr;
}

CVMTNode::~CVMTNode() {
    delete[] mName;
}

void CVMTNode::SetName(const char *name) {
    delete[] mName;
    mName = new char[strlen(name) + 1];
    strcpy(mName, name);
}

const char *CVMTNode::GetName() const {
    return mName;
}

CVMTGroupNode *CVMTNode::GetParent() const {
    return mParent;
}
