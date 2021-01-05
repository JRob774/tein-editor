// Some more descriptive keywords for static depending on the context being used.
#define Persistent static
#define Internal static
#define Global static

// We use static linkage for faster compilation times as we use unity/jumbo build.
// It's a define so that if we decide to change this then the process is easy.
#define EditorAPI static

typedef  uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef   int8_t  S8;
typedef  int16_t S16;
typedef  int32_t S32;
typedef  int64_t S64;

// @Refactor: Do we need all of the functionality of glm? It's quite a large library
// and we are probably only going to use a small subset so we might want to just
// implement that functionality ourselves (for now it is just useful to have though).
typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;
typedef glm::mat2 Mat2;
typedef glm::mat3 Mat3;
typedef glm::mat4 Mat4;

struct Rect
{
    float x,y,w,h;

    inline bool operator== (const Rect& rhs) const {
        return ((x == rhs.x) && (y == rhs.y) && (w == rhs.w) && (h == rhs.h));
    }
    inline bool operator!= (const Rect& rhs) const {
        return !(operator==(rhs));
    }
};

// We wrap the C functions malloc and free because C++ does not implicitly cast from void* so this macro handles the cast.
#define Allocate(t,sz) static_cast<t*>(malloc((sz)*sizeof(t)))
#define Deallocate(pt) free((pt))

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
const auto& DeferJoin(defer, __LINE__) = DeferHelp() + [&]()
#endif

template<typename T>
struct DeferType
{
    T lambda;

    DeferType (T lambda): lambda(lambda) {}
   ~DeferType () { lambda(); }

    // No copy!
    DeferType& operator= (const DeferType& d) = delete;
    DeferType (const DeferType& d) = delete;
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

///////////////////////
// Utility Functions //
///////////////////////

EditorAPI std::vector<U8> ReadEntireBinaryFile (std::string fileName);
EditorAPI std::string ReadEntireTextFile (std::string fileName);

EditorAPI void TokenizeString (const std::string& str, const char* delims, std::vector<std::string>& tokens);

EditorAPI std::string FormatString (const char* format, ...);
EditorAPI std::string FormatString_V (const char* format, va_list args);

EditorAPI bool StringCaseInsensitiveCompare (const std::string& a, const std::string& b);

// These functions have platform-specific implementations in the source file.
EditorAPI std::string GetExecutablePath ();
EditorAPI bool RunExecutable (std::string exe);
EditorAPI void LoadWebpage (std::string url);
