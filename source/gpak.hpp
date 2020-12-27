/*******************************************************************************
 * Facilities for packing and unpacking file data from the GPAK file format.
 * Authored by Joshua Robertson
 * Available Under MIT License (See EOF)
 *
*******************************************************************************/

#pragma once

/*////////////////////////////////////////////////////////////////////////////*/

/* -------------------------------------------------------------------------- */

// The GPAK system returns error codes as we don't want to call the error log
// system on another thread. So the returned code tells us which error to log.

enum class GPAK_Error
{
    NONE,   // Successfully packed/unpacked GPAK.
    WRITE,  // Failed to open the GPAK for writing.
    READ,   // Failed to open the GPAK for reading.
    EMPTY,  // No files were found in paths to pack.
};

struct GPAK_Entry
{
    U16  name_length;
    std::string name;
    U32    file_size;
};

/* -------------------------------------------------------------------------- */

TEINAPI void gpak_unpack (std::string file_name, bool overwrite);
TEINAPI void gpak_pack   (std::string file_name, std::vector<std::string> paths);

TEINAPI float gpak_unpack_progress ();
TEINAPI float gpak_pack_progress   ();

TEINAPI bool is_gpak_unpack_complete ();
TEINAPI bool is_gpak_pack_complete   ();

TEINAPI void do_unpack ();
TEINAPI void do_pack   ();

TEINAPI void cancel_unpack ();
TEINAPI void cancel_pack   ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
