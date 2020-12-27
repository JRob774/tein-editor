#if defined(PLATFORM_WIN32)
FILDEF HWND internal__win32_get_window_handle (SDL_Window* window)
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
#endif

STDDEF std::vector<u8> read_binary_file (std::string file_name)
{
    std::ifstream file(file_name, std::ios::binary);
    std::vector<u8> content;
    content.resize(get_size_of_file(file_name));
    file.read(reinterpret_cast<char*>(&content[0]), content.size()*sizeof(u8));
    return content;
}

STDDEF std::string read_entire_file (std::string file_name)
{
    std::ifstream file(file_name);
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

#if defined(PLATFORM_WIN32)
STDDEF std::string get_executable_path ()
{
    constexpr size_t EXECUTABLE_BUFFER_SIZE = MAX_PATH+1;
    char temp_buffer[EXECUTABLE_BUFFER_SIZE] = {};

    GetModuleFileNameA(NULL, temp_buffer, EXECUTABLE_BUFFER_SIZE);
    std::string path(fix_path_slashes(temp_buffer));

    // Get rid of the actual executable so it's just the path.
    size_t last_slash = path.find_last_of('/');
    if (last_slash != std::string::npos) ++last_slash;

    return path.substr(0, last_slash);
}
#else
#error get_executable_path not implemented on the current platform!
#endif

FILDEF size_t get_size_of_file (std::string file_name)
{
    FILE* file = fopen(file_name.c_str(), "rb");
    if (!file) return 0;
    defer { fclose(file); };
    fseek(file, 0L, SEEK_END);
    return ftell(file);
}
FILDEF size_t get_size_of_file (FILE* file)
{
    if (!file) return 0;
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file); // Go back to avoid changing stream.
    return size;
}

#if defined(PLATFORM_WIN32)
FILDEF bool does_file_exist (std::string file_name)
{
    DWORD attributes = GetFileAttributesA(file_name.c_str());
    return ((attributes != INVALID_FILE_ATTRIBUTES) &&
           !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}
#else
#error does_file_exist not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
FILDEF bool does_path_exist (std::string path_name)
{
    DWORD attributes = GetFileAttributesA(path_name.c_str());
    return ((attributes != INVALID_FILE_ATTRIBUTES) &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY));
}
#else
#error does_path_exist not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
STDDEF void list_path_content (std::string path_name, std::vector<std::string>& content)
{
    // Clean the path in case there are trailing slashes.
    path_name = fix_path_slashes(path_name);
    while (path_name.back() == '/') path_name.pop_back();

    if (!is_path(path_name)) return;

    std::string find_path(path_name + "\\*");
    WIN32_FIND_DATAA find_data = {};

    HANDLE find_file = FindFirstFileA(find_path.c_str(), &find_data);
    defer { FindClose(find_file); };

    if (find_file != INVALID_HANDLE_VALUE)
    {
        do
        {
            // We do not want to include the self and parent directories.
            std::string file_name(find_data.cFileName);
            if (file_name != "." && file_name != "..")
            {
                content.push_back(path_name + "/" + fix_path_slashes(file_name));
            }
        }
        while (FindNextFile(find_file, &find_data));
    }
}
#else
#error list_path_content not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
STDDEF void list_path_content_r (std::string path_name, std::vector<std::string>& content)
{
    // Clean the path in case there are trailing slashes.
    path_name = fix_path_slashes(path_name);
    while (path_name.back() == '/') path_name.pop_back();

    if (!is_path(path_name)) return;

    std::string find_path(path_name + "\\*");
    WIN32_FIND_DATAA find_data = {};

    HANDLE find_file = FindFirstFileA(find_path.c_str(), &find_data);
    defer { FindClose(find_file); };

    if (find_file != INVALID_HANDLE_VALUE)
    {
        do
        {
            // We do not want to include the self and parent directories.
            std::string file_name(find_data.cFileName);
            if (file_name != "." && file_name != "..")
            {
                content.push_back(path_name + "/" + fix_path_slashes(file_name));
                if (is_path(content.back()))
                {
                    list_path_content_r(content.back(), content);
                }
            }
        }
        while (FindNextFile(find_file, &find_data));
    }
}
#else
#error list_path_content_r not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
STDDEF void list_path_files (std::string path_name, std::vector<std::string>& files)
{
    // Clean the path in case there are trailing slashes.
    path_name = fix_path_slashes(path_name);
    while (path_name.back() == '/') path_name.pop_back();

    if (!is_path(path_name)) return;

    std::string find_path(path_name + "\\*");
    WIN32_FIND_DATAA find_data = {};

    HANDLE find_file = FindFirstFileA(find_path.c_str(), &find_data);
    defer { FindClose(find_file); };

    if (find_file != INVALID_HANDLE_VALUE)
    {
        do
        {
            // We do not want to include the self and parent directories.
            std::string file_name(find_data.cFileName);
            if (file_name != "." && file_name != "..")
            {
                std::string final(path_name + "/" + fix_path_slashes(file_name));
                if (is_file(final)) files.push_back(final);
            }
        }
        while (FindNextFile(find_file, &find_data));
    }
}
#else
#error list_path_files not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
STDDEF void list_path_files_r (std::string path_name, std::vector<std::string>& files)
{
    // Clean the path in case there are trailing slashes.
    path_name = fix_path_slashes(path_name);
    while (path_name.back() == '/') path_name.pop_back();

    if (!is_path(path_name)) return;

    std::string find_path(path_name + "\\*");
    WIN32_FIND_DATAA find_data = {};

    HANDLE find_file = FindFirstFileA(find_path.c_str(), &find_data);
    defer { FindClose(find_file); };

    if (find_file != INVALID_HANDLE_VALUE)
    {
        do
        {
            // We do not want to include the self and parent directories.
            std::string file_name(find_data.cFileName);
            if (file_name != "." && file_name != "..")
            {
                std::string final(path_name + "/" + fix_path_slashes(file_name));
                if (is_file(final)) files.push_back(final);
                else list_path_files_r(final, files);
            }
        }
        while (FindNextFile(find_file, &find_data));
    }
}
#else
#error list_path_files_r not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
FILDEF bool create_path (std::string path_name)
{
    std::vector<std::string> paths;
    tokenize_string(path_name, "\\/", paths);

    if (!paths.empty())
    {
        std::string path;
        for (auto& p: paths)
        {
            path += (p + "/");
            if (!does_path_exist(path))
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
#else
#error create_path not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
FILDEF bool is_path_absolute (std::string path_name)
{
    return !PathIsRelativeA(path_name.c_str());
}
#else
#error is_path_absoolute not implemented on the current platform!
#endif

// Aliases of the previous functions because the naming makes better sense in context.

FILDEF bool is_file (std::string file_name)
{
    return does_file_exist(file_name);
}

FILDEF bool is_path (std::string path_name)
{
    return does_path_exist(path_name);
}

#if defined(PLATFORM_WIN32)
FILDEF u64 last_file_write_time (std::string file_name)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    ULARGE_INTEGER write_time = {};
    if (GetFileAttributesExA(file_name.c_str(), GetFileExInfoStandard, &attributes))
    {
        write_time.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
        write_time.LowPart  = attributes.ftLastWriteTime.dwLowDateTime;
    }
    return write_time.QuadPart;
}
#else
#error last_file_write_time not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
FILDEF int compare_file_write_times (u64 a, u64 b)
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
#else
#error compare_file_write_times not implemented on the current platform!
#endif

FILDEF std::string make_path_absolute (std::string path_name)
{
    if (!is_path_absolute(path_name)) path_name.insert(0, get_executable_path());
    return path_name;
}

FILDEF std::string fix_path_slashes (std::string path_name)
{
    std::replace(path_name.begin(), path_name.end(), '\\', '/');
    return path_name;
}

FILDEF std::string strip_file_path (std::string file_name)
{
    file_name = fix_path_slashes(file_name);
    size_t last_slash = file_name.rfind('/');
    if (last_slash != std::string::npos)
    {
        if (last_slash == file_name.length()) file_name.clear();
        else file_name = file_name.substr(last_slash+1);
    }
    return file_name;
}

FILDEF std::string strip_file_ext (std::string file_name)
{
    file_name = fix_path_slashes(file_name);
    size_t last_dot = file_name.rfind('.');
    if (last_dot != std::string::npos)
    {
        if (last_dot == 0) file_name.clear();
        else file_name = file_name.substr(0, last_dot);
    }
    return file_name;
}

FILDEF std::string strip_file_name (std::string file_name)
{
    file_name = fix_path_slashes(file_name);
    size_t last_slash = file_name.rfind('/');
    if (last_slash != std::string::npos && last_slash != file_name.length())
    {
        file_name = file_name.substr(0, last_slash+1);
    }
    return file_name;
}

FILDEF std::string strip_file_path_and_ext (std::string file_name)
{
    return strip_file_ext(strip_file_path(file_name));
}

FILDEF void tokenize_string (const std::string& str, const char* delims,
                             std::vector<std::string>& tokens)
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

INLDEF std::string format_string (const char* format, ...)
{
    va_list args;

            va_start(args, format);
    defer { va_end  (args); };

    return format_string_v(format, args);
}

INLDEF std::string format_string_v (const char* format, va_list args)
{
    std::string str;
    int size = vsnprintf(NULL, 0, format, args) + 1;
    char* buffer = cstd_malloc(char, size);
    if (buffer)
    {
        vsnprintf(buffer, size, format, args);
        str = buffer;
        cstd_free(buffer);
    }
    return str;
}

FILDEF vec2 get_mouse_pos ()
{
    int imx, imy;
    SDL_GetMouseState(&imx, &imy);
    return vec2(imx,imy);
}

INLDEF std::string format_time (const char* format)
{
    time_t     raw_time = time(NULL);
    struct tm* cur_time = localtime(&raw_time);

    size_t length = 256;
    size_t result = 0;

    // We go until our buffer is big enough.
    char* buffer = NULL;
    do
    {
        if (buffer) cstd_free(buffer);
        buffer = cstd_malloc(char, length);
        if (!buffer) return std::string();

        result = strftime(buffer, length, format, cur_time);
        length *= 2;
    }
    while (!result);

    defer { cstd_free(buffer); };
    return std::string(buffer);
}

#if defined(PLATFORM_WIN32)
FILDEF unsigned int get_thread_id ()
{
    return GetCurrentThreadId();
}
#else
#error get_thread_id not implemented on the current platform!
#endif

FILDEF bool point_in_bounds_xyxy (vec2 p, quad q)
{
    return (p.x >= q.x1 && p.y >= q.y1 && p.x <= q.x2 && p.y <= q.y2);
}

FILDEF bool point_in_bounds_xywh (vec2 p, quad q)
{
    return (p.x >= q.x && p.y >= q.y && p.x < (q.x+q.w) && p.y < (q.y+q.h));
}

FILDEF bool insensitive_compare (const std::string& a, const std::string& b)
{
    if (a.length() != b.length()) return false;
    for (std::string::size_type i=0; i<a.length(); ++i)
    {
        if (tolower(a[i]) != tolower(b[i])) return false;
    }
    return true;
}

FILDEF bool string_replace (std::string& str, const std::string& from, const std::string& to)
{
    std::string::size_type start_pos = str.find(from);
    if (start_pos == std::string::npos) return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

#if defined(PLATFORM_WIN32)
FILDEF bool run_executable (std::string exe)
{
    PROCESS_INFORMATION process_info = {};
    STARTUPINFOA        startup_info = {};

    startup_info.cb = sizeof(STARTUPINFOA);

    if (!CreateProcessA(exe.c_str(), NULL,NULL,NULL, FALSE, 0, NULL,
        strip_file_name(exe).c_str(), &startup_info, &process_info))
    {
        return false;
    }

    // Win32 API docs state these should be closed.
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    return true;
}
#else
#error run_executable not implemented on the current platform!
#endif

#if defined(PLATFORM_WIN32)
FILDEF void load_webpage (std::string url)
{
    ShellExecuteA(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOW);
}
#else
#error load_webpage not implemented on the current platform!
#endif
