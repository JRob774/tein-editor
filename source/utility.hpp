#pragma once

#define INTERNAL static
#define GLOBAL static

#define STDDEF INTERNAL        // @Temporary: Rename this!
#define INLDEF INTERNAL inline // @Temporary: Remove this!
#define FILDEF INTERNAL inline // @Temporary: Remove this!

#define JOIN( a, b) JOIN2(a, b)
#define JOIN2(a, b) JOIN1(a, b)
#define JOIN1(a, b) a##b

#ifdef __COUNTER__
#define defer \
const auto& JOIN(defer, __COUNTER__) = Defer_Help__() + [&]()
#else
#define defer \
const auto& JOIN(defer,    __LINE__) = Defer_Help__() + [&]()
#endif

template<typename T>
struct Defer__
{
    T lambda;

               Defer__ (T lambda): lambda(lambda) { /* Nothing! */ }
              ~Defer__ ()                         {    lambda();   }

               Defer__ (const Defer__& object) = delete;
    Defer__& operator= (const Defer__& object) = delete;
};
struct Defer_Help__
{
    template<typename T>
    Defer__<T> operator+ (T type) { return type; }
};

// @Temporary: Just replace with the actual assert() function call!
#if defined(BUILD_DEBUG)
#include <cassert>
#define ASSERT(e) assert(e)
#else
#define ASSERT(e) ((void)0)
#endif

#define cstd_malloc( t,     sz) (t*)malloc ((sz)     *sizeof(t))
#define cstd_realloc(t, pt, sz) (t*)realloc((pt),(sz)*sizeof(t))
#define cstd_calloc( t,     sz) (t*)calloc ((sz),     sizeof(t))
#define cstd_free(      pt)           free ((pt))

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

STDDEF std::vector<u8> read_binary_file (std::string file_name);
STDDEF std::string     read_entire_file (std::string file_name);

STDDEF std::string get_executable_path ();

FILDEF size_t get_size_of_file (std::string file_name);
FILDEF size_t get_size_of_file (FILE*       file);

FILDEF bool does_file_exist (std::string file_name);
FILDEF bool does_path_exist (std::string path_name);

STDDEF void list_path_content   (std::string path_name, std::vector<std::string>& content);
STDDEF void list_path_content_r (std::string path_name, std::vector<std::string>& content);
STDDEF void list_path_files     (std::string path_name, std::vector<std::string>& files  );
STDDEF void list_path_files_r   (std::string path_name, std::vector<std::string>& files  );

FILDEF bool create_path (std::string path_name);

FILDEF bool is_path_absolute (std::string path_name);

FILDEF bool is_file (std::string file_name);
FILDEF bool is_path (std::string path_name);

FILDEF u64 last_file_write_time (std::string file_name);

FILDEF int compare_file_write_times (u64 a, u64 b);

FILDEF std::string make_path_absolute (std::string path_name);
FILDEF std::string fix_path_slashes   (std::string path_name);

FILDEF std::string strip_file_path         (std::string file_name);
FILDEF std::string strip_file_ext          (std::string file_name);
FILDEF std::string strip_file_name         (std::string file_name);
FILDEF std::string strip_file_path_and_ext (std::string file_name);

FILDEF void tokenize_string (const std::string& str, const char* delims,
                             std::vector<std::string>& tokens);

INLDEF std::string format_string   (const char* format, ...);
INLDEF std::string format_string_v (const char* format, va_list args);

FILDEF vec2 get_mouse_pos ();

INLDEF std::string format_time (const char* format);

FILDEF unsigned int get_thread_id ();

FILDEF bool point_in_bounds_xyxy (vec2 p, quad q);
FILDEF bool point_in_bounds_xywh (vec2 p, quad q);

FILDEF bool insensitive_compare (const std::string& a, const std::string& b);

FILDEF bool string_replace (std::string& str, const std::string& from, const std::string& to);

FILDEF bool run_executable (std::string exe);
FILDEF void load_webpage   (std::string url);
