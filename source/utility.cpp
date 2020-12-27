namespace Internal
{
    #if defined(PLATFORM_WIN32)
    TEINAPI HWND Win32GetWindowHandle (SDL_Window* window)
    {
        SDL_SysWMinfo win_info = {};
        SDL_VERSION(&win_info.version);
        HWND hwnd = NULL;
        if (SDL_GetWindowWMInfo(window, &win_info))
        {
            hwnd = win_info.info.win.window;;
        }
        return hwnd;
    }
    #endif // PLATFORM_WIN32
}

TEINAPI std::vector<U8> ReadBinaryFile (std::string fileName)
{
    std::ifstream file(fileName, std::ios::binary);
    std::vector<U8> content;
    content.resize(GetSizeOfFile(fileName));
    file.read(reinterpret_cast<char*>(&content[0]), content.size()*sizeof(U8));
    return content;
}

TEINAPI std::string ReadEntireFile (std::string fileName)
{
    std::ifstream file(fileName);
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

#if defined(PLATFORM_WIN32)
TEINAPI std::string GetExecutablePath ()
{
    constexpr size_t EXECUTABLE_BUFFER_SIZE = MAX_PATH+1;
    char tempBuffer[EXECUTABLE_BUFFER_SIZE] = {};

    GetModuleFileNameA(NULL, tempBuffer, EXECUTABLE_BUFFER_SIZE);
    std::string path(FixPathSlashes(tempBuffer));

    // Get rid of the actual executable so it's just the path.
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos) ++lastSlash;

    return path.substr(0, lastSlash);
}
#endif // PLATFORM_WIN32

TEINAPI size_t GetSizeOfFile (std::string fileName)
{
    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file) return 0;
    Defer { fclose(file); };
    fseek(file, 0L, SEEK_END);
    return ftell(file);
}
TEINAPI size_t GetSizeOfFile (FILE* file)
{
    if (!file) return 0;
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file); // Go back to avoid changing stream.
    return size;
}

#if defined(PLATFORM_WIN32)
TEINAPI bool DoesFileExist (std::string fileName)
{
    DWORD attribs = GetFileAttributesA(fileName.c_str());
    return ((attribs != INVALID_FILE_ATTRIBUTES) && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI bool DoesPathExist (std::string pathName)
{
    DWORD attribs = GetFileAttributesA(pathName.c_str());
    return ((attribs != INVALID_FILE_ATTRIBUTES) && (attribs & FILE_ATTRIBUTE_DIRECTORY));
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI void ListPathContent (std::string pathName, std::vector<std::string>& content, bool recursive)
{
    // Clean the path in case there are trailing slashes.
    pathName = FixPathSlashes(pathName);
    while (pathName.back() == '/') pathName.pop_back();

    if (!IsPath(pathName)) return;

    std::string findPath(pathName + "\\*");
    WIN32_FIND_DATAA findData = {};

    HANDLE findFile = FindFirstFileA(findPath.c_str(), &findData);
    Defer { FindClose(findFile); };

    if (findFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            // We do not want to include the self and parent directories.
            std::string fileName(findData.cFileName);
            if (fileName != "." && fileName != "..")
            {
                content.push_back(pathName + "/" + FixPathSlashes(fileName));
                if (recursive && IsPath(content.back()))
                {
                    ListPathContent(content.back(), content, recursive);
                }
            }
        }
        while (FindNextFile(findFile, &findData));
    }
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI void ListPathFiles (std::string pathName, std::vector<std::string>& files, bool recursive)
{
    // Clean the path in case there are trailing slashes.
    pathName = FixPathSlashes(pathName);
    while (pathName.back() == '/') pathName.pop_back();

    if (!IsPath(pathName)) return;

    std::string findPath(pathName + "\\*");
    WIN32_FIND_DATAA findData = {};

    HANDLE findFile = FindFirstFileA(findPath.c_str(), &findData);
    Defer { FindClose(findFile); };

    if (findFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            // We do not want to include the self and parent directories.
            std::string fileName(findData.cFileName);
            if (fileName != "." && fileName != "..")
            {
                std::string final(pathName + "/" + FixPathSlashes(fileName));
                if (IsFile(final)) files.push_back(final);
                else if (recursive) ListPathFiles(final, files, recursive);
            }
        }
        while (FindNextFile(findFile, &findData));
    }
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI bool CreatePath (std::string pathName)
{
    std::vector<std::string> paths;
    TokenizeString(pathName, "\\/", paths);

    if (!paths.empty())
    {
        std::string path;
        for (auto& p: paths)
        {
            path += (p + "/");
            if (!DoesPathExist(path))
            {
                if (!CreateDirectoryA(path.c_str(), NULL))
                {
                    return false;
                }
            }
        }
        return true;
    }

    return false;
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI bool IsPathAbsolute (std::string pathName)
{
    return !PathIsRelativeA(pathName.c_str());
}
#endif // PLATFORM_WIN32

// Aliases of the previous functions because the naming makes better sense in context.

TEINAPI bool IsFile (std::string fileName)
{
    return DoesFileExist(fileName);
}

TEINAPI bool IsPath (std::string pathName)
{
    return DoesPathExist(pathName);
}

#if defined(PLATFORM_WIN32)
TEINAPI U64 LastFileWriteTime (std::string fileName)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    ULARGE_INTEGER writeTime = {};
    if (GetFileAttributesExA(fileName.c_str(), GetFileExInfoStandard, &attributes))
    {
        writeTime.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
        writeTime.LowPart = attributes.ftLastWriteTime.dwLowDateTime;
    }
    return writeTime.QuadPart;
}
#endif // PLATFORM_wIN32

#if defined(PLATFORM_WIN32)
TEINAPI int CompareFileWriteTimes (U64 a, U64 b)
{
    ULARGE_INTEGER a2, b2;
    FILETIME a3, b3;

    a2.QuadPart = a;
    b2.QuadPart = b;

    a3.dwHighDateTime = a2.HighPart;
    a3.dwLowDateTime = a2.LowPart;
    b3.dwHighDateTime = b2.HighPart;
    b3.dwLowDateTime = b2.LowPart;

    return CompareFileTime(&a3, &b3);
}
#endif // PLATFORM_WIN32

TEINAPI std::string MakePathAbsolute (std::string pathName)
{
    return (!IsPathAbsolute(pathName)) ? pathName.insert(0, GetExecutablePath()) : pathName;
}

TEINAPI std::string FixPathSlashes (std::string pathName)
{
    std::replace(pathName.begin(), pathName.end(), '\\', '/');
    return pathName;
}

TEINAPI std::string StripFilePath (std::string fileName)
{
    fileName = FixPathSlashes(fileName);
    size_t lastSlash = fileName.rfind('/');
    if (lastSlash != std::string::npos)
    {
        if (lastSlash == fileName.length()) fileName.clear();
        else fileName = fileName.substr(lastSlash+1);
    }
    return fileName;
}

TEINAPI std::string StripFileExt (std::string fileName)
{
    fileName = FixPathSlashes(fileName);
    size_t lastDot = fileName.rfind('.');
    if (lastDot != std::string::npos)
    {
        if (lastDot == 0) fileName.clear();
        else fileName = fileName.substr(0, lastDot);
    }
    return fileName;
}

TEINAPI std::string StripFileName (std::string fileName)
{
    fileName = FixPathSlashes(fileName);
    size_t lastSlash = fileName.rfind('/');
    if (lastSlash != std::string::npos && lastSlash != fileName.length())
    {
        fileName = fileName.substr(0, lastSlash+1);
    }
    return fileName;
}

TEINAPI std::string StripFilePathAndExt (std::string fileName)
{
    return StripFileExt(StripFilePath(fileName));
}

TEINAPI void TokenizeString (const std::string& str, const char* delims, std::vector<std::string>& tokens)
{
    size_t prev = 0;
    size_t pos;

    while ((pos = str.find_first_of(delims, prev)) != std::string::npos)
    {
        if (pos > prev) tokens.push_back(str.substr(prev, pos-prev));
        prev = pos+1;
    }
    if (prev < str.length())
    {
        tokens.push_back(str.substr(prev, std::string::npos));
    }
}

TEINAPI std::string FormatString (const char* format, ...)
{
    va_list args;
    va_start(args, format);
    Defer { va_end(args); };
    return FormatStringV(format, args);
}

TEINAPI std::string FormatStringV (const char* format, va_list args)
{
    std::string str;
    int size = vsnprintf(NULL, 0, format, args) + 1;
    char* buffer = Malloc(char, size);
    if (buffer)
    {
        vsnprintf(buffer, size, format, args);
        str = buffer;
        Free(buffer);
    }
    return str;
}

TEINAPI Vec2 GetMousePos ()
{
    int imx,imy;
    SDL_GetMouseState(&imx,&imy);
    return Vec2(imx,imy);
}

TEINAPI std::string FormatTime (const char* format)
{
    time_t raw_time = time(NULL);
    struct tm* cur_time = localtime(&raw_time);

    size_t length = 256;
    size_t result = 0;

    // We go until our buffer is big enough.
    char* buffer = NULL;
    do
    {
        if (buffer) Free(buffer);
        buffer = Malloc(char, length);
        if (!buffer) return std::string();

        result = strftime(buffer, length, format, cur_time);
        length *= 2;
    }
    while (!result);

    Defer { Free(buffer); };
    return std::string(buffer);
}

#if defined(PLATFORM_WIN32)
TEINAPI unsigned int GetThreadID ()
{
    return GetCurrentThreadId();
}
#endif // PLATFORM_WIN32

TEINAPI bool PointInBoundsXYXY (Vec2 p, Quad q)
{
    return (p.x >= q.x1 && p.y >= q.y1 && p.x <= q.x2 && p.y <= q.y2);
}

TEINAPI bool PointInBoundsXYWH (Vec2 p, Quad q)
{
    return (p.x >= q.x && p.y >= q.y && p.x < (q.x+q.w) && p.y < (q.y+q.h));
}

TEINAPI bool InsensitiveCompare (const std::string& a, const std::string& b)
{
    if (a.length() != b.length()) return false;
    for (std::string::size_type i=0; i<a.length(); ++i)
    {
        if (tolower(a[i]) != tolower(b[i])) return false;
    }
    return true;
}

TEINAPI bool StringReplace (std::string& str, const std::string& from, const std::string& to)
{
    std::string::size_type startPos = str.find(from);
    if (startPos == std::string::npos) return false;
    str.replace(startPos, from.length(), to);
    return true;
}

#if defined(PLATFORM_WIN32)
TEINAPI bool RunExecutable (std::string exe)
{
    PROCESS_INFORMATION processInfo = {};
    STARTUPINFOA startupInfo = {};

    startupInfo.cb = sizeof(startupInfo);

    if (!CreateProcessA(exe.c_str(), NULL,NULL,NULL, FALSE, 0, NULL, StripFileName(exe).c_str(), &startupInfo, &processInfo))
    {
        return false;
    }

    // Win32 API docs state these should be closed.
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return true;
}
#endif // PLATFORM_WIN32

#if defined(PLATFORM_WIN32)
TEINAPI void LoadWebpage (std::string url)
{
    ShellExecuteA(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOW);
}
#endif
