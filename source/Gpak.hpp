// The GPAK system returns error codes as we don't want to call the error log
// system on another thread. So the returned code tells us which error to log.
enum class GPAKError
{
    NONE,   // Successfully packed/unpacked GPAK.
    WRITE,  // Failed to open the GPAK for writing.
    READ,   // Failed to open the GPAK for reading.
    EMPTY,  // No files were found in paths to pack.
};

struct GPAKEntry
{
    U16 nameLength;
    std::string name;
    U32 fileSize;
};

TEINAPI void GPAKUnpack (std::string fileName, bool overwrite);
TEINAPI void GPAKPack   (std::string fileName, std::vector<std::string> paths);

TEINAPI float GPAKUnpackProgress ();
TEINAPI float GPAKPackProgress   ();

TEINAPI bool IsGPAKUnpackComplete ();
TEINAPI bool IsGPAKPackComplete   ();

TEINAPI void DoUnpack ();
TEINAPI void DoPack   ();

TEINAPI void CancelUnpack ();
TEINAPI void CancelPack   ();
