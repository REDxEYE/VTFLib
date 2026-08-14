/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include <deque>

#include "VTFLib.h"
#include "VMTWrapper.h"
#include "VMTFile.h"

using namespace VTFLib;

namespace VTFLib {
    std::deque<int32_t> CurrentIndex;
    Nodes::CVMTGroupNode *CurrentNode = nullptr;
}

//
// vlMaterialBound()
// Returns true if an material is bound, false otherwise.
//
VTFLIB_API vlBool vlMaterialIsBound(Diagnostics::CError &error) {
    if (!bInitialized) {
        VTFError_Set(error, "VTFLib not initialized.");
        return vlFalse;
    }

    return Material != nullptr;
}

//
// vlBindMaterial()
// Bind a material to operate on.
// All library routines will use this material.
//
VTFLIB_API vlBool vlBindMaterial(uint32_t uiMaterial, Diagnostics::CError &error) {
    if (!bInitialized) {
        VTFError_Set(error, "VTFLib not initialized.");
        return vlFalse;
    }

    if (uiMaterial >= MaterialVector->size() || (*MaterialVector)[uiMaterial] == nullptr) {
        VTFError_Set(error, "Invalid material.");
        return vlFalse;
    }

    if (Material == (*MaterialVector)[uiMaterial]) // If it is already bound do nothing.
        return vlTrue;

    Material = (*MaterialVector)[uiMaterial];

    CurrentIndex.clear();
    CurrentNode = nullptr;

    return vlTrue;
}

//
// vlCreateMaterial()
// Create an material to work on.
//
VTFLIB_API vlBool vlCreateMaterial(uint32_t *uiMaterial, Diagnostics::CError &error) {
    if (!bInitialized) {
        VTFError_Set(error, "VTFLib not initialized.");
        return vlFalse;
    }

    MaterialVector->push_back(new CVMTFile());
    *uiMaterial = (uint32_t) MaterialVector->size() - 1;

    return vlTrue;
}

//
// vlDeleteMaterial()
// Delete a material and all resources associated with it.
//

VTFLIB_API void vlDeleteMaterial(uint32_t uiMaterial) {
    if (!bInitialized)
        return;

    if (uiMaterial >= MaterialVector->size())
        return;

    if ((*MaterialVector)[uiMaterial] == nullptr)
        return;

    if ((*MaterialVector)[uiMaterial] == Material) {
        Material = nullptr;

        CurrentIndex.clear();
        CurrentNode = nullptr;
    }

    delete (*MaterialVector)[uiMaterial];
    (*MaterialVector)[uiMaterial] = nullptr;
}

VTFLIB_API vlBool vlMaterialCreate(const char *cRoot, Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    CurrentIndex.clear();
    CurrentNode = nullptr;

    return Material->Create(cRoot);
}

VTFLIB_API void vlMaterialDestroy() {
    if (Material == nullptr) {
        return;
    }

    CurrentIndex.clear();
    CurrentNode = nullptr;

    Material->Destroy();
}

VTFLIB_API vlBool vlMaterialIsLoaded(Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    return Material->IsLoaded();
}

VTFLIB_API vlBool vlMaterialLoad(const char *cFileName, Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    CurrentIndex.clear();
    CurrentNode = nullptr;

    return Material->Load(cFileName, error);
}

VTFLIB_API vlBool vlMaterialLoadLump(const void *lpData, uint32_t uiBufferSize, Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    CurrentIndex.clear();
    CurrentNode = nullptr;

    return Material->Load(lpData, uiBufferSize, error);
}

VTFLIB_API vlBool vlMaterialLoadProc(void *pUserData, Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    CurrentIndex.clear();
    CurrentNode = nullptr;

    return Material->Load(pUserData, error);
}

VTFLIB_API vlBool vlMaterialSave(const char *cFileName, Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    return Material->Save(cFileName, error);
}

VTFLIB_API vlBool vlMaterialSaveLump(void *lpData, ssize_t uiBufferSize, ssize_t *uiSize,
                                     Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    return Material->Save(lpData, uiBufferSize, *uiSize, error);
}

VTFLIB_API vlBool vlMaterialSaveProc(void *pUserData, Diagnostics::CError &error) {
    if (Material == nullptr) {
        VTFError_Set(error, "No material bound.");
        return vlFalse;
    }

    return Material->Save(pUserData, error);
}

//
// vlMaterialGetCurretNode()
// Gets the current node in the transversal.
//
Nodes::CVMTNode *vlMaterialGetCurretNode() {
    if (Material == nullptr || CurrentNode == nullptr)
        return nullptr;

    int32_t iIndex = CurrentIndex.back();

    if (iIndex == -1 || iIndex == (int32_t) CurrentNode->GetNodeCount()) {
        return CurrentNode;
    }

    return CurrentNode->GetNode((uint32_t) iIndex);
}

//
// vlMaterialGetCurretNode()
// Gets the current node type in the transversal.
//
VMTNodeType vlMaterialGetCurretNodeType() {
    if (Material == nullptr || CurrentNode == nullptr)
        return NODE_TYPE_COUNT;

    int32_t iIndex = CurrentIndex.back();

    if (iIndex == -1) {
        return NODE_TYPE_GROUP;
    }

    if (iIndex == (int32_t) CurrentNode->GetNodeCount()) {
        return NODE_TYPE_GROUP_END;
    }

    return CurrentNode->GetNode((uint32_t) iIndex)->GetType();
}

//
// vlMaterialGetFirstNode()
// Moves the current node to the stat of the root node.
//
VTFLIB_API vlBool vlMaterialGetFirstNode() {
    if (Material == nullptr || Material->GetRoot() == nullptr)
        return vlFalse;

    CurrentNode = Material->GetRoot();
    CurrentIndex.push_back(-1);

    return vlTrue;
}

//
// vlMaterialGetLastNode()
// Moves the current node to the end of the root node.
//
VTFLIB_API vlBool vlMaterialGetLastNode() {
    if (Material == nullptr || Material->GetRoot() == nullptr)
        return vlFalse;

    CurrentNode = Material->GetRoot();
    CurrentIndex.push_back(CurrentNode->GetNodeCount());

    return vlTrue;
}

//
// vlMaterialGetNextNode()
// Moves the current node to the next node depth first style.
//
VTFLIB_API vlBool vlMaterialGetNextNode() {
    if (Material == nullptr || CurrentNode == nullptr)
        return vlFalse;

    // If we are at the end of the current node, go up a level.
    if (CurrentIndex.back() == (int32_t) CurrentNode->GetNodeCount()) {
        if (CurrentNode->GetParent() != nullptr) {
            CurrentNode = CurrentNode->GetParent();
            CurrentIndex.pop_back();

            return vlTrue;
        } else {
            return vlFalse;
        }
    }

    // Go to the next node in the current node.
    CurrentIndex.back()++;

    // Check if we are at the end.
    if (CurrentIndex.back() == (int32_t) CurrentNode->GetNodeCount()) {
        return vlTrue;
    }

    Nodes::CVMTNode *VMTNode = CurrentNode->GetNode((uint32_t) CurrentIndex.back());

    // If the current node is a group, enter it at the start.
    if (VMTNode->GetType() == NODE_TYPE_GROUP) {
        CurrentNode = static_cast<Nodes::CVMTGroupNode *>(VMTNode);
        CurrentIndex.push_back(-1);
    }

    return vlTrue;
}

//
// vlMaterialGetPreviousNode()
// Moves the current node to the previous node depth first style.  This
// is the reverse of vlMaterialGetNextNode().
//
VTFLIB_API vlBool vlMaterialGetPreviousNode() {
    if (Material == nullptr || CurrentNode == nullptr)
        return vlFalse;

    // If we are at the start of the current node, go up a level.
    if (CurrentIndex.back() == -1) {
        if (CurrentNode->GetParent() != nullptr) {
            CurrentNode = CurrentNode->GetParent();
            CurrentIndex.pop_back();

            return vlTrue;
        } else {
            return vlFalse;
        }
    }

    // Go to the previous node in the current node.
    CurrentIndex.back()--;

    // Check if we are at the start.
    if (CurrentIndex.back() == -1) {
        return vlTrue;
    }

    Nodes::CVMTNode *VMTNode = CurrentNode->GetNode((uint32_t) CurrentIndex.back());

    // If the current node is a group, enter it at the end.
    if (VMTNode->GetType() == NODE_TYPE_GROUP) {
        CurrentNode = static_cast<Nodes::CVMTGroupNode *>(VMTNode);
        CurrentIndex.push_back(CurrentNode->GetNodeCount());
    }

    return vlTrue;
}

//
// vlMaterialGetParentNode()
// Moves the current node to the current node's parent.
//
VTFLIB_API vlBool vlMaterialGetParentNode() {
    if (Material == nullptr || CurrentNode == nullptr)
        return vlFalse;

    // If we are not the root node, go up a level.
    if (CurrentNode->GetParent() != nullptr) {
        CurrentNode = CurrentNode->GetParent();
        CurrentIndex.pop_back();

        return vlTrue;
    }

    return vlFalse;
}

//
// vlMaterialGetParentNode()
// Moves the current node to the specified child node of the current node.
//
VTFLIB_API vlBool vlMaterialGetChildNode(const char *cName) {
    if (Material == nullptr || CurrentNode == nullptr)
        return vlFalse;

    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    // Only groups have children.
    if (VMTNode->GetType() != NODE_TYPE_GROUP)
        return vlFalse;

    Nodes::CVMTGroupNode *VMTGroupNode = static_cast<Nodes::CVMTGroupNode *>(VMTNode);

    // Search for the specified child.
    for (uint32_t i = 0; i < VMTGroupNode->GetNodeCount(); i++) {
        VMTNode = VMTGroupNode->GetNode(i);
        if (_stricmp(VMTNode->GetName(), cName) == 0) {
            // If the child is a group, enter it at the start.
            if (VMTNode->GetType() == NODE_TYPE_GROUP) {
                CurrentNode = static_cast<Nodes::CVMTGroupNode *>(VMTNode);
                CurrentIndex.push_back(-1);
            } else {
                CurrentIndex.back() = (int32_t) i;
            }

            return vlTrue;
        }
    }

    return vlFalse;
}

//
// vlMaterialGetNodeName()
// Gets the current node's name.
//
VTFLIB_API const char *vlMaterialGetNodeName() {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode == nullptr)
        return nullptr;

    return VMTNode->GetName();
}

//
// vlMaterialSetNodeName()
// Sets the current node's name.
//
VTFLIB_API void vlMaterialSetNodeName(const char *cName) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode == nullptr)
        return;

    VMTNode->SetName(cName);
}

//
// GetNodeType()
// Get the type of node.  Returns VL_NODE_UNKOWN at the end of a group.
// For example, subsequent calls to vlGetNextNode() may return the following
// structure.  After the last VL_NODE_UNKOWN, vlGetNextNode() will fail.
//
// NODE_TYPE_GROUP
//   NODE_TYPE_GROUP
//     NODE_TYPE_STRING
//     NODE_TYPE_INTEGER
//   NODE_TYPE_GROUP_END
//   NODE_TYPE_GROUP
//     NODE_TYPE_SINGLE
//     NODE_TYPE_SINGLE
//   NODE_TYPE_GROUP_END
//   NODE_TYPE_STRING
// NODE_TYPE_GROUP_END
//
VTFLIB_API VMTNodeType vlMaterialGetNodeType() {
    return vlMaterialGetCurretNodeType();
}

//
// vlMaterialGetNodeString()
// If the current node is a string node, this gets its value.
//
VTFLIB_API const char *vlMaterialGetNodeString() {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_STRING)
        return nullptr;

    return static_cast<Nodes::CVMTStringNode *>(VMTNode)->GetValue();
}

//
// vlMaterialSetNodeString()
// If the current node is a string node, this sets its value.
//
VTFLIB_API void vlMaterialSetNodeString(const char *cValue) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_STRING)
        return;

    static_cast<Nodes::CVMTStringNode *>(VMTNode)->SetValue(cValue);
}

//
// vlMaterialGetNodeInteger()
// If the current node is a integer node, this gets its value.
//
VTFLIB_API uint32_t vlMaterialGetNodeInteger() {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_INTEGER)
        return 0;

    return static_cast<Nodes::CVMTIntegerNode *>(VMTNode)->GetValue();
}

//
// vlMaterialSetNodeInteger()
// If the current node is a integer node, this sets its value.
//
VTFLIB_API void vlMaterialSetNodeInteger(uint32_t iValue) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_INTEGER)
        return;

    static_cast<Nodes::CVMTIntegerNode *>(VMTNode)->SetValue(iValue);
}

//
// vlMaterialGetNodeSingle()
// If the current node is a single node, this gets its value.
//
VTFLIB_API float vlMaterialGetNodeSingle() {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_SINGLE)
        return 0.0f;

    return static_cast<Nodes::CVMTSingleNode *>(VMTNode)->GetValue();
}

//
// vlMaterialSetNodeSingle()
// If the current node is a single node, this sets its value.
//
VTFLIB_API void vlMaterialSetNodeSingle(float sValue) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_SINGLE)
        return;

    static_cast<Nodes::CVMTSingleNode *>(VMTNode)->SetValue(sValue);
}

//
// vlMaterialAddNodeGroup()
// If the current node is a group node, this adds a group node to the current node.
//
VTFLIB_API void vlMaterialAddNodeGroup(const char *cName) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_GROUP)
        return;

    static_cast<Nodes::CVMTGroupNode *>(VMTNode)->AddGroupNode(cName);
}

//
// vlMaterialAddNodeString()
// If the current node is a group node, this adds a string node to the current node.
//
VTFLIB_API void vlMaterialAddNodeString(const char *cName, const char *cValue) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_GROUP)
        return;

    static_cast<Nodes::CVMTGroupNode *>(VMTNode)->AddStringNode(cName, cValue);
}

//
// vlMaterialAddNodeInteger()
// If the current node is a group node, this adds a integer node to the current node.
//
VTFLIB_API void vlMaterialAddNodeInteger(const char *cName, uint32_t iValue) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_GROUP)
        return;

    static_cast<Nodes::CVMTGroupNode *>(VMTNode)->AddIntegerNode(cName, iValue);
}

//
// vlMaterialAddNodeSingle()
// If the current node is a group node, this adds a single node to the current node.
//
VTFLIB_API void vlMaterialAddNodeSingle(const char *cName, float sValue) {
    Nodes::CVMTNode *VMTNode = vlMaterialGetCurretNode();

    if (VMTNode->GetType() != NODE_TYPE_GROUP)
        return;

    static_cast<Nodes::CVMTGroupNode *>(VMTNode)->AddSingleNode(cName, sValue);
}
