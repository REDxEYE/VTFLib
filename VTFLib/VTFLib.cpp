/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VTFLib.h"
#include "VTFFile.h"
#include "VMTFile.h"

using namespace VTFLib;

namespace VTFLib
{
	vlBool bInitialized = vlFalse;

	CVTFFile *Image = nullptr;
	CImageVector *ImageVector = nullptr;

	CVMTFile *Material = nullptr;
	CMaterialVector *MaterialVector = nullptr;

	float sLuminanceWeightR = 0.299f;
	float sLuminanceWeightG = 0.587f;
	float sLuminanceWeightB = 0.114f;

	uint16_t uiBlueScreenMaskR = 0x0000;
	uint16_t uiBlueScreenMaskG = 0x0000;
	uint16_t uiBlueScreenMaskB = 0xffff;

	uint16_t uiBlueScreenClearR = 0x0000;
	uint16_t uiBlueScreenClearG = 0x0000;
	uint16_t uiBlueScreenClearB = 0x0000;

	float sFP16HDRExposure = 2.0f;

	uint32_t uiVMTParseMode = PARSE_MODE_LOOSE;
}

//
// vlGetVersion()
// Gets the library's version number.
//
VTFLIB_API uint32_t vlGetVersion()
{
	return VL_VERSION;
}

//
// vlGetVersionString()
// Gets the library's version number string.
//
VTFLIB_API const char *vlGetVersionString()
{
	return VL_VERSION_STRING;
}



//
// vlInitialize()
// Initializes all resources.
//
VTFLIB_API vlBool vlInitialize(Diagnostics::CError& error)
{
	if(bInitialized)
	{
		VTFError_Set(error, "VTFLib already initialized.");
		return vlFalse;
	}

	bInitialized = vlTrue;

	ImageVector = new CImageVector();
	MaterialVector = new CMaterialVector();

	return vlTrue;
}

//
// vlShutdown()
// Frees all resources.
//
VTFLIB_API void vlShutdown()
{
	if(!bInitialized)
		return;

	uint32_t i;

	bInitialized = vlFalse;

	Image = nullptr;
	Material = nullptr;

	for(i = 0; i < ImageVector->size(); i++)
	{
		delete (*ImageVector)[i];
	}

	delete ImageVector;
	ImageVector = nullptr;

	for(i = 0; i < MaterialVector->size(); i++)
	{
		delete (*MaterialVector)[i];
	}

	delete MaterialVector;
	MaterialVector = nullptr;
}

VTFLIB_API vlBool vlGetBoolean(VTFLibOption Option)
{
	return vlFalse;
}

VTFLIB_API void vlSetBoolean(VTFLibOption Option, vlBool bValue)
{

}

VTFLIB_API int32_t vlGetInteger(VTFLibOption Option)
{
	switch(Option) {
		case VTFLIB_BLUESCREEN_MASK_R:
			return (int32_t)uiBlueScreenMaskR;
		case VTFLIB_BLUESCREEN_MASK_G:
			return (int32_t)uiBlueScreenMaskG;
		case VTFLIB_BLUESCREEN_MASK_B:
			return (int32_t)uiBlueScreenMaskB;

		case VTFLIB_BLUESCREEN_CLEAR_R:
			return (int32_t)uiBlueScreenClearR;
		case VTFLIB_BLUESCREEN_CLEAR_G:
			return (int32_t)uiBlueScreenClearG;
		case VTFLIB_BLUESCREEN_CLEAR_B:
			return (int32_t)uiBlueScreenClearB;

		case VTFLIB_VMT_PARSE_MODE:
			return (int32_t)uiVMTParseMode;
	}

	return 0;
}

VTFLIB_API void vlSetInteger(VTFLibOption Option, int32_t iValue)
{
	switch(Option)
	{
	case VTFLIB_BLUESCREEN_MASK_R:
		if(iValue < 0)
			iValue = 0;
		else if(iValue > 65535)
			iValue = 65535;
		uiBlueScreenMaskR = (uint16_t)iValue;
		break;
	case VTFLIB_BLUESCREEN_MASK_G:
		if(iValue < 0)
			iValue = 0;
		else if(iValue > 65535)
			iValue = 65535;
		uiBlueScreenMaskG = (uint16_t)iValue;
		break;
	case VTFLIB_BLUESCREEN_MASK_B:
		if(iValue < 0)
			iValue = 0;
		else if(iValue > 65535)
			iValue = 65535;
		uiBlueScreenMaskB = (uint16_t)iValue;
		break;

	case VTFLIB_BLUESCREEN_CLEAR_R:
		if(iValue < 0)
			iValue = 0;
		else if(iValue > 65535)
			iValue = 65535;
		uiBlueScreenClearR = (uint16_t)iValue;
		break;
	case VTFLIB_BLUESCREEN_CLEAR_G:
		if(iValue < 0)
			iValue = 0;
		else if(iValue > 65535)
			iValue = 65535;
		uiBlueScreenClearG = (uint16_t)iValue;
		break;
	case VTFLIB_BLUESCREEN_CLEAR_B:
		if(iValue < 0)
			iValue = 0;
		else if(iValue > 65535)
			iValue = 65535;
		uiBlueScreenClearB = (uint16_t)iValue;
		break;

	case VTFLIB_VMT_PARSE_MODE:
		if(iValue < 0 || iValue >= PARSE_MODE_COUNT)
			return;
		uiVMTParseMode = (uint32_t)iValue;
		break;
	}
}

VTFLIB_API float vlGetFloat(VTFLibOption Option)
{
	switch(Option)
	{
	case VTFLIB_LUMINANCE_WEIGHT_R:
		return sLuminanceWeightR;
	case VTFLIB_LUMINANCE_WEIGHT_G:
		return sLuminanceWeightG;
	case VTFLIB_LUMINANCE_WEIGHT_B:
		return sLuminanceWeightB;

	case VTFLIB_FP16_HDR_EXPOSURE:
		return sFP16HDRExposure;
	}

	return 0.0f;
}

VTFLIB_API void vlSetFloat(VTFLibOption Option, float sValue)
{
	switch(Option)
	{
	case VTFLIB_LUMINANCE_WEIGHT_R:
		if(sValue < 0.0f)
			sValue = 0.0f;
		sLuminanceWeightR = sValue;
		break;
	case VTFLIB_LUMINANCE_WEIGHT_G:
		if(sValue < 0.0f)
			sValue = 0.0f;
		sLuminanceWeightG = sValue;
		break;
	case VTFLIB_LUMINANCE_WEIGHT_B:
		if(sValue < 0.0f)
			sValue = 0.0f;
		sLuminanceWeightB = sValue;
		break;

	case VTFLIB_FP16_HDR_EXPOSURE:
		sFP16HDRExposure = sValue;
		break;
	}
}

//
// DllMain()
// DLL entry point.
//
#ifdef _WIN32

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			vlShutdown();
			break;
	}

	return TRUE;
}

#else

__attribute__((destructor))
static void VTFLibUnload()
{
	vlShutdown();
}

#endif