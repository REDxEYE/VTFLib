/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VMTGroupNode.h"

#include <algorithm>

using namespace VTFLib::Nodes;

CVMTGroupNode::CVMTGroupNode(const char *name) : CVMTNode(name) {
    mVMTNodeList = new CVMTNodeList();
}

CVMTGroupNode::CVMTGroupNode(const CVMTGroupNode &other) : CVMTNode(other.GetName()) {
    mVMTNodeList = new CVMTNodeList();
    for (const auto &it: *other.mVMTNodeList) {
        AddNode(it->Clone());
    }
}

CVMTGroupNode::~CVMTGroupNode() {
    for (const auto &it: *mVMTNodeList) {
        delete it;
    }
    delete mVMTNodeList;
}

VMTNodeType CVMTGroupNode::GetType() const {
    return NODE_TYPE_GROUP;
}

size_t CVMTGroupNode::GetNodeCount() const {
    return mVMTNodeList->size();
}

CVMTNode *CVMTGroupNode::AddNode(CVMTNode *node) {
    // We can do this because we are friends.
    node->mParent = this;
    mVMTNodeList->push_back(node);
    return node;
}

CVMTGroupNode *CVMTGroupNode::AddGroupNode(const char *name) {
    auto *group = new CVMTGroupNode(name);
    AddNode(group);
    return group;
}

CVMTStringNode *CVMTGroupNode::AddStringNode(const char *name, const char *value) {
    auto *strNode = new CVMTStringNode(name, value);
    AddNode(strNode);
    return strNode;
}

CVMTIntegerNode *CVMTGroupNode::AddIntegerNode(const char *name, const int32_t value) {
    auto *intNode = new CVMTIntegerNode(name, value);
    AddNode(intNode);
    return intNode;
}

CVMTSingleNode *CVMTGroupNode::AddSingleNode(const char *name, const float value) {
    auto *floatNode = new CVMTSingleNode(name, value);
    AddNode(floatNode);
    return floatNode;
}

void CVMTGroupNode::RemoveNode(const CVMTNode *node) const {
    const auto it = std::find(mVMTNodeList->begin(), mVMTNodeList->end(), node);
    if (it == mVMTNodeList->end())
        return;

    delete *it;
    mVMTNodeList->erase(it);
}

void CVMTGroupNode::RemoveAllNodes() const {
    for (const auto &it: *mVMTNodeList) {
        delete it;
    }

    mVMTNodeList->clear();
}

CVMTNode *CVMTGroupNode::GetNode(const size_t index) const {
    size_t count = 0;
    for (const auto &it: *mVMTNodeList) {
        if (count == index) {
            return it;
        }
        count++;
    }

    return nullptr;
}

CVMTNode *CVMTGroupNode::GetNode(const char *name) const {
    for (const auto &it: *mVMTNodeList) {
        if (stricmp(name, it->GetName()) == 0) {
            return it;
        }
    }

    return nullptr;
}

CVMTNode *CVMTGroupNode::Clone() const {
    return new CVMTGroupNode(*this);
}
