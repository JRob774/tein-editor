/*******************************************************************************
 * Functionality for creating simplistic custom alert message box popups.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

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

TEINAPI int show_alert (std::string title, std::string msg, int type, int buttons, std::string window = "");

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
