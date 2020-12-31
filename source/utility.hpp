// We use static linkage for faster compilation times as we use unity/jumbo build.
#define TEINAPI static

typedef  uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef   int8_t  S8;
typedef  int16_t S16;
typedef  int32_t S32;
typedef  int64_t S64;

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

// A macro for generating bitflag operators for enumerations as they do not work by default in C++.
// The implementation of this macro is based off of winnt.h's DEFINE_ENUM_FLAG_OPERATORS() macro.

template<size_t S> struct EnumIntegralSizeType;

template<> struct EnumIntegralSizeType<1> { typedef  S8 type; };
template<> struct EnumIntegralSizeType<2> { typedef S16 type; };
template<> struct EnumIntegralSizeType<4> { typedef S32 type; };
template<> struct EnumIntegralSizeType<8> { typedef S64 type; };

template<typename T> struct GetEnumIntegralSizeType
{
    typedef typename EnumIntegralSizeType<sizeof(T)>::type type;
};

#define GenerateEnumBitflagOperators(t)                                                                                              \
inline t& operator |= (t& a, t b) { return (t&)( (GetEnumIntegralSizeType<t>::type&)(a) |= (GetEnumIntegralSizeType<t>::type)(b)); } \
inline t& operator &= (t& a, t b) { return (t&)( (GetEnumIntegralSizeType<t>::type&)(a) &= (GetEnumIntegralSizeType<t>::type)(b)); } \
inline t& operator ^= (t& a, t b) { return (t&)( (GetEnumIntegralSizeType<t>::type&)(a) ^= (GetEnumIntegralSizeType<t>::type)(b)); } \
inline t  operator |  (t  a, t b) { return (t )( (GetEnumIntegralSizeType<t>::type )(a) |  (GetEnumIntegralSizeType<t>::type)(b)); } \
inline t  operator &  (t  a, t b) { return (t )( (GetEnumIntegralSizeType<t>::type )(a) &  (GetEnumIntegralSizeType<t>::type)(b)); } \
inline t  operator ^  (t  a, t b) { return (t )( (GetEnumIntegralSizeType<t>::type )(a) ^  (GetEnumIntegralSizeType<t>::type)(b)); } \
inline t  operator ~  (t  a     ) { return (t )(~(GetEnumIntegralSizeType<t>::type )(a));                                          }

// We wrap the C functions malloc and free because C++ does not implicitly cast from void* so this macro handles the cast.
#define Malloc(t,sz) static_cast<t*>(malloc((sz)*sizeof(t)))
#define Free(pt) free((pt))

typedef glm::vec2   Vec2;
typedef glm::vec3   Vec3;
typedef glm::vec4   Vec4;
typedef glm::ivec2 IVec2;
typedef glm::ivec3 IVec3;
typedef glm::ivec4 IVec4;
typedef glm::ivec4 IVec4;
typedef glm::mat2   Mat2;
typedef glm::mat3   Mat3;
typedef glm::mat4   Mat4;

struct Quad
{
    union { float x, x1; };
    union { float y, y1; };
    union { float w, x2; };
    union { float h, y2; };
};

enum AlertButton
{
    ALERT_BUTTON_YES_NO_CANCEL,
    ALERT_BUTTON_YES_NO,
    ALERT_BUTTON_OK,
    ALERT_BUTTON_OK_CANCEL
};
enum AlertType
{
    ALERT_TYPE_INFO,
    ALERT_TYPE_WARNING,
    ALERT_TYPE_ERROR
};
enum AlertResult
{
    ALERT_RESULT_INVALID,
    ALERT_RESULT_CANCEL,
    ALERT_RESULT_OK,
    ALERT_RESULT_NO,
    ALERT_RESULT_YES
};

TEINAPI std::vector<U8> ReadBinaryFile (std::string fileName);
TEINAPI std::string     ReadEntireFile (std::string fileName);

TEINAPI std::string GetExecutablePath ();

TEINAPI size_t GetSizeOfFile (std::string fileName);
TEINAPI size_t GetSizeOfFile (FILE* file);

TEINAPI bool DoesFileExist (std::string fileName);
TEINAPI bool DoesPathExist (std::string pathName);

TEINAPI void ListPathContent (std::string pathName, std::vector<std::string>& content, bool recursive = false);
TEINAPI void ListPathFiles   (std::string pathName, std::vector<std::string>& files,   bool recursive = false);

TEINAPI bool CreatePath (std::string pathName);

TEINAPI bool IsPathAbsolute (std::string pathName);

TEINAPI bool IsFile (std::string fileName);
TEINAPI bool IsPath (std::string pathName);

TEINAPI U64 LastFileWriteTime (std::string fileName);

TEINAPI int CompareFileWriteTimes (U64 a, U64 b);

TEINAPI std::string MakePathAbsolute (std::string pathName);
TEINAPI std::string FixPathSlashes   (std::string pathName);

TEINAPI std::string StripFilePath       (std::string fileName);
TEINAPI std::string StripFileExt        (std::string fileName);
TEINAPI std::string StripFileName       (std::string fileName);
TEINAPI std::string StripFilePathAndExt (std::string fileName);

TEINAPI void TokenizeString (const std::string& str, const char* delims, std::vector<std::string>& tokens);

TEINAPI std::string FormatString  (const char* format, ...);
TEINAPI std::string FormatStringV (const char* format, va_list args);

TEINAPI Vec2 GetMousePos ();

TEINAPI std::string FormatTime (const char* format);

TEINAPI unsigned int GetThreadID ();

TEINAPI bool PointInBoundsXYXY (Vec2 p, Quad q);
TEINAPI bool PointInBoundsXYWH (Vec2 p, Quad q);

TEINAPI bool InsensitiveCompare (const std::string& a, const std::string& b);

TEINAPI bool StringReplace (std::string& str, const std::string& from, const std::string& to);

TEINAPI bool RunExecutable (std::string exe);
TEINAPI void LoadWebpage   (std::string url);

TEINAPI AlertResult ShowAlert (std::string title, std::string msg, AlertType type, AlertButton buttons, std::string window = "");
