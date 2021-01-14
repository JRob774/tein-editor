#pragma once

#include <cstdlib>
#include <cstdint>

#include <filesystem>

#include <string>
#include <vector>

#include <glm/glm.hpp>

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

// A macro that wraps some boilerplate for generating a nicely scoped enumeration.

#define DECLARE_ENUM(scope, name, type) \
namespace scope { enum name: type; }    \
typedef type name;                      \
enum scope::name: type

namespace TEIN
{
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

    // Internal implementation details of the Defer macro.
    template<typename T>
    struct DeferType
    {
        DeferType (T lambda): lambda(lambda) {}
       ~DeferType () { lambda(); }
        // No copy!
        DeferType& operator= (const DeferType& d) = delete;
        DeferType (const DeferType& d) = delete;

        T lambda;
    };
    struct DeferHelp
    {
        template<typename T>
        DeferType<T> operator+ (T type) { return type; }
    };

    namespace Utility
    {
        // We wrap the C functions malloc and free because C++ does not
        // implicitly cast from void* so this handles the cast for us.
        template<typename T>
        inline T* Allocate (size_t count)
        {
            return static_cast<T*>(malloc(count*sizeof(T)));
        }
        inline void Deallocate (void* ptr)
        {
            free(ptr);
        }

        std::vector<U8> ReadEntireBinaryFile (std::string fileName);
        std::string ReadEntireTextFile (std::string fileName);

        void TokenizeString (const std::string& str, const char* delims, std::vector<std::string>& tokens);

        std::string FormatString (const char* format, ...);
        std::string FormatString_V (const char* format, va_list args);

        bool StringCaseInsensitiveCompare (const std::string& a, const std::string& b);

        // These functions have platform-specific implementations in the source file.
        std::filesystem::path GetExecutablePath ();
        std::filesystem::path GetSaveDataPath ();
        bool RunExecutable (std::string exe);
        void LoadWebpage (std::string url);
    }
}
