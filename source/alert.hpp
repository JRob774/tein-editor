/*******************************************************************************
 * Functionality for creating simplistic custom alert message box popups.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#ifndef ALERT_HPP__ /*////////////////////////////////////////////////////////*/
#define ALERT_HPP__

#ifdef COMPILER_HAS_PRAGMA_ONCE
#pragma once
#endif

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
static constexpr int ALERT_BUTTON_YES_NO_CANCEL = MB_YESNOCANCEL;
static constexpr int ALERT_BUTTON_YES_NO        = MB_YESNO;
static constexpr int ALERT_BUTTON_OK            = MB_OK;
static constexpr int ALERT_BUTTON_OK_CANCEL     = MB_OKCANCEL;
#else
#error ALERT_BUTTON enumeration not defined!
#endif

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
static constexpr int ALERT_TYPE_INFO    = MB_ICONINFORMATION;
static constexpr int ALERT_TYPE_WARNING = MB_ICONWARNING;
static constexpr int ALERT_TYPE_ERROR   = MB_ICONERROR;
#else
#error ALERT_TYPE enumeration not defined!
#endif

/* -------------------------------------------------------------------------- */

#if defined(PLATFORM_WIN32)
static constexpr int ALERT_RESULT_INVALID = 0;
static constexpr int ALERT_RESULT_CANCEL  = IDCANCEL;
static constexpr int ALERT_RESULT_OK      = IDOK;
static constexpr int ALERT_RESULT_NO      = IDNO;
static constexpr int ALERT_RESULT_YES     = IDYES;
#else
#error ALERT_RESULT enumeration not defined!
#endif

/* -------------------------------------------------------------------------- */

STDDEF int show_alert (std::string title, std::string msg, int type,
                       int buttons, std::string window = "");

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/

#endif /* ALERT_HPP__ ////////////////////////////////////////////////////////*/

/*******************************************************************************
 *
 * Copyright (c) 2020 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
