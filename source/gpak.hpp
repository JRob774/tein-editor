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
    u16  name_length;
    std::string name;
    u32    file_size;
};

/* -------------------------------------------------------------------------- */

FILDEF void gpak_unpack (std::string file_name, bool overwrite);
FILDEF void gpak_pack   (std::string file_name, std::vector<std::string> paths);

FILDEF float gpak_unpack_progress ();
FILDEF float gpak_pack_progress   ();

FILDEF bool is_gpak_unpack_complete ();
FILDEF bool is_gpak_pack_complete   ();

FILDEF void do_unpack ();
FILDEF void do_pack   ();

FILDEF void cancel_unpack ();
FILDEF void cancel_pack   ();

/* -------------------------------------------------------------------------- */

/*////////////////////////////////////////////////////////////////////////////*/
