#pragma once

// We use static linkage for faster compilation times as we use unity/jumbo build.
#define TEINAPI static

// C++ implementation of defer functionality. This can be used to defer blocks of
// code to be executed during exit of the current scope. Useful for freeing any
// resources that have been allocated without worrying about multiple paths or
// having to deal with C++'s RAII model for implementing this kind of behaviour.

#define DeferJoin( a, b) DeferJoin2(a, b)
#define DeferJoin2(a, b) DeferJoin1(a, b)
#define DeferJoin1(a, b) a##b

#ifdef __COUNTER__
#define Defer \
const auto& DeferJoin(defer, __COUNTER__) = DeferHelp() + [&]()
#else
#define Defer \
const auto& DeferJoin(defer,    __LINE__) = DeferHelp() + [&]()
#endif

template<typename T>
struct DeferType
{
    T lambda;

    DeferType (T lambda): lambda(lambda) { /* Nothing! */ }
   ~DeferType ()                         {    lambda();   }

               DeferType (const DeferType& d) = delete;
    DeferType& operator= (const DeferType& d) = delete;
};
struct DeferHelp
{
    template<typename T>
    DeferType<T> operator+ (T type) { return type; }
};

#define Malloc(t,sz) static_cast<t*>(malloc((sz)*sizeof(t)))
#define Free(pt) free((pt))

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef   int8_t  s8;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;

typedef glm::vec2   vec2;
typedef glm::vec3   vec3;
typedef glm::vec4   vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::ivec4 ivec4;
typedef glm::mat2   mat2;
typedef glm::mat3   mat3;
typedef glm::mat4   mat4;

struct quad
{
    union { float x, x1; };
    union { float y, y1; };
    union { float w, x2; };
    union { float h, y2; };
};

TEINAPI std::vector<u8> read_binary_file (std::string file_name);
TEINAPI std::string     read_entire_file (std::string file_name);

TEINAPI std::string get_executable_path ();

TEINAPI size_t get_size_of_file (std::string file_name);
TEINAPI size_t get_size_of_file (FILE*       file);

TEINAPI bool does_file_exist (std::string file_name);
TEINAPI bool does_path_exist (std::string path_name);

TEINAPI void list_path_content (std::string path_name, std::vector<std::string>& content, bool recursive = false);
TEINAPI void list_path_files   (std::string path_name, std::vector<std::string>& files,   bool recursive = false);

TEINAPI bool create_path (std::string path_name);

TEINAPI bool is_path_absolute (std::string path_name);

TEINAPI bool is_file (std::string file_name);
TEINAPI bool is_path (std::string path_name);

TEINAPI u64 last_file_write_time (std::string file_name);

TEINAPI int compare_file_write_times (u64 a, u64 b);

TEINAPI std::string make_path_absolute (std::string path_name);
TEINAPI std::string fix_path_slashes   (std::string path_name);

TEINAPI std::string strip_file_path         (std::string file_name);
TEINAPI std::string strip_file_ext          (std::string file_name);
TEINAPI std::string strip_file_name         (std::string file_name);
TEINAPI std::string strip_file_path_and_ext (std::string file_name);

TEINAPI void tokenize_string (const std::string& str, const char* delims,
                             std::vector<std::string>& tokens);

TEINAPI std::string format_string   (const char* format, ...);
TEINAPI std::string format_string_v (const char* format, va_list args);

TEINAPI vec2 get_mouse_pos ();

TEINAPI std::string format_time (const char* format);

TEINAPI unsigned int get_thread_id ();

TEINAPI bool point_in_bounds_xyxy (vec2 p, quad q);
TEINAPI bool point_in_bounds_xywh (vec2 p, quad q);

TEINAPI bool insensitive_compare (const std::string& a, const std::string& b);

TEINAPI bool string_replace (std::string& str, const std::string& from, const std::string& to);

TEINAPI bool run_executable (std::string exe);
TEINAPI void load_webpage   (std::string url);
