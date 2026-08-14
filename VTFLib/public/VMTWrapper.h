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

#ifdef __cplusplus
extern "C" {
#endif

//
// Memory managment routines.
//

VTFLIB_API vlBool vlMaterialIsBound(VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlBindMaterial(uint32_t uiMaterial, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlCreateMaterial(uint32_t *uiMaterial, VTFLib::Diagnostics::CError& error);
VTFLIB_API void vlDeleteMaterial(uint32_t uiMaterial);

//
// Library routines.  (Basically class wrappers.)
//

VTFLIB_API vlBool vlMaterialCreate(const char *cRoot, VTFLib::Diagnostics::CError& error);
VTFLIB_API void vlMaterialDestroy();

VTFLIB_API vlBool vlMaterialIsLoaded(VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlMaterialLoad(const char *cFileName, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlMaterialLoadLump(const void *lpData, uint32_t uiBufferSize, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlMaterialLoadProc(void *pUserData, VTFLib::Diagnostics::CError& error);

VTFLIB_API vlBool vlMaterialSave(const char *cFileName, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlMaterialSaveLump(void *lpData, ssize_t uiBufferSize, ssize_t *uiSize, VTFLib::Diagnostics::CError& error);
VTFLIB_API vlBool vlMaterialSaveProc(void *pUserData, VTFLib::Diagnostics::CError& error);

//
// Node routines.
//

VTFLIB_API vlBool vlMaterialGetFirstNode();
VTFLIB_API vlBool vlMaterialGetLastNode();
VTFLIB_API vlBool vlMaterialGetNextNode();
VTFLIB_API vlBool vlMaterialGetPreviousNode();

VTFLIB_API vlBool vlMaterialGetParentNode();
VTFLIB_API vlBool vlMaterialGetChildNode(const char *cName);

VTFLIB_API const char *vlMaterialGetNodeName();
VTFLIB_API void vlMaterialSetNodeName(const char *cName);

VTFLIB_API VMTNodeType vlMaterialGetNodeType();

VTFLIB_API const char *vlMaterialGetNodeString();
VTFLIB_API void vlMaterialSetNodeString(const char *cValue);

VTFLIB_API uint32_t vlMaterialGetNodeInteger();
VTFLIB_API void vlMaterialSetNodeInteger(uint32_t iValue);

VTFLIB_API float vlMaterialGetNodeSingle();
VTFLIB_API void vlMaterialSetNodeSingle(float sValue);

VTFLIB_API void vlMaterialAddNodeGroup(const char *cName);
VTFLIB_API void vlMaterialAddNodeString(const char *cName, const char *cValue);
VTFLIB_API void vlMaterialAddNodeInteger(const char *cName, uint32_t iValue);
VTFLIB_API void vlMaterialAddNodeSingle(const char *cName, float sValue);

#ifdef __cplusplus
}
#endif

