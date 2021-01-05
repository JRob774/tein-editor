EditorAPI std::vector<U8> ReadEntireBinaryFile (std::string fileName)
{
    std::ifstream file(fileName, std::ios::binary);
    std::vector<U8> content;
    content.resize(std::filesystem::file_size(fileName));
    file.read(reinterpret_cast<char*>(&content[0]), content.size()*sizeof(U8));
    return content;
}
EditorAPI std::string ReadEntireTextFile (std::string fileName)
{
    std::ifstream file(fileName);
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

EditorAPI void TokenizeString (const std::string& str, const char* delims, std::vector<std::string>& tokens)
{
    size_t prev = 0;
    size_t pos;

    while ((pos = str.find_first_of(delims, prev)) != std::string::npos) {
        if (pos > prev) tokens.push_back(str.substr(prev, pos-prev));
        prev = pos+1;
    }
    if (prev < str.length()) {
        tokens.push_back(str.substr(prev, std::string::npos));
    }
}

EditorAPI std::string FormatString (const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string str = FormatString_V(format, args);
    va_end(args);
    return str;
}
EditorAPI std::string FormatString_V (const char* format, va_list args)
{
    std::string str;
    int size = vsnprintf(NULL, 0, format, args) + 1;
    char* buffer = Allocate(char, size);
    if (buffer) {
        Defer { Deallocate(buffer); };
        vsnprintf(buffer, size, format, args);
        str = buffer;
    }
    return str;
}

EditorAPI bool StringCaseInsensitiveCompare (const std::string& a, const std::string& b)
{
    if (a.length() != b.length()) return false;
    for (std::string::size_type i=0, n=a.length(); i<n; ++i) {
        if (tolower(a[i]) != tolower(b[i])) return false;
    }
    return true;
}

// The following are Utility functions that require platform-specific implementations.

#if defined(PLATFORM_WIN32)

EditorAPI std::string GetExecutablePath ()
{
    // On return remove the executable name so it's just the path.
    char pathBuffer[MAX_PATH] = {};
    GetModuleFileNameA(NULL, pathBuffer, MAX_PATH);
    return std::filesystem::path(pathBuffer).remove_filename().string();
}

EditorAPI std::string GetSaveDataPath ()
{
    char pathBuffer[MAX_PATH] = {};
    std::filesystem::path path;
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pathBuffer))) {
        path = pathBuffer;
        path.append("TheEndEditor");
    }
    return path.string();
}

EditorAPI bool RunExecutable (std::string exe)
{
    STARTUPINFOA startupInfo = { sizeof(startupInfo) };
    PROCESS_INFORMATION processInfo = {};
    if (std::filesystem::is_regular_file(exe)) {
        if (!CreateProcessA(exe.c_str(), NULL,NULL,NULL, FALSE, 0, NULL,
            std::filesystem::path(exe).remove_filename().string().c_str(), &startupInfo, &processInfo)) {
            return false;
        }
    } else {
        return false;
    }
    // Win32 API docs state these should be closed.
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return true;
}

EditorAPI void LoadWebpage (std::string url)
{
    // MSDN recommends initializing the COM like this before using ShellExecute.
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
    ShellExecuteA(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOW);
    CoUninitialize();
}

#endif // PLATFORM_WIN32
