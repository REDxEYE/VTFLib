/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

// ============================================================
// NOTE: This file is commented for compatibility with Doxygen.
// ============================================================
/*!
	\file Error.h
	\brief Error handling class header.
*/

#pragma once

#include "VTFLibShared.h"


namespace VTFLib::Diagnostics {
    //! VTFLib Error handling class
    /*!
        The Error handling class allows you to access a text description
        for the last error encountered.
    */
    class CError {
    public:
        CError();

        ~CError();

        //! Clear the error message buffer.
        void Clear();

        //! Get the error message text.
        [[nodiscard]] const char *Get() const;

        //! Set the error message buffer.
        void SetFormatted(const char *format, ...);

        void Set(const char *errorMessage, bool systemError = vlFalse);

        [[nodiscard]] bool isSet() const;

    private:
        char *mErrorMessage;
    };
}

#define VTFError_Set(error, message) error.SetFormatted("[%s:%i:%s]: %s",__FILE__, __LINE__, __FUNCTION__, message)
#define VTFError_Set_SE(error, message) error.SetFormatted("[%s:%i:%s]: %s",__FILE__, __LINE__, __FUNCTION__, message, true)
#define VTFError_Set_Formatted(error, format, ...) error.SetFormatted("[%s:%i:%s]: " #format ,__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


