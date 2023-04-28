#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H

#include "flatbuffers/base.h"
#include "flatbuffers/util.h"

// clang-format off

#ifdef __ANDROID__
  #include <android/log.h>
  #define TEST_OUTPUT_LINE(...) \
      __android_log_print(ANDROID_LOG_INFO, "FlatBuffers", __VA_ARGS__)
  #define FLATBUFFERS_NO_FILE_TESTS
#else
  #define TEST_OUTPUT_LINE(...) \
      do { printf(__VA_ARGS__); printf("\n"); } while(!IsConstTrue(true))
#endif

#define TEST_EQ(exp, val) TestEq(exp, val, "'" #exp "' != '" #val "'", __FILE__, __LINE__, "")
#define TEST_NE(exp, val) TestNe(exp, val, "'" #exp "' == '" #val "'", __FILE__, __LINE__, "")
#define TEST_ASSERT(val)  TestEq(true, !!(val), "'" "true" "' != '" #val "'", __FILE__, __LINE__, "")
#define TEST_NOTNULL(val) TestEq(true, (val) != nullptr, "'" "nullptr" "' == '" #val "'", __FILE__, __LINE__, "")
#define TEST_EQ_STR(exp, val) TestEqStr(exp, val, "'" #exp "' != '" #val "'", __FILE__, __LINE__, "")

#ifdef _WIN32
  #define TEST_ASSERT_FUNC(val) TestEq(true, !!(val), "'" "true" "' != '" #val "'", __FILE__, __LINE__, __FUNCTION__)
  #define TEST_EQ_FUNC(exp, val) TestEq(exp, val, "'" #exp "' != '" #val "'", __FILE__, __LINE__, __FUNCTION__)
#else
  #define TEST_ASSERT_FUNC(val) TestEq(true, !!(val), "'" "true" "' != '" #val "'", __FILE__, __LINE__, __PRETTY_FUNCTION__)
  #define TEST_EQ_FUNC(exp, val) TestEq(exp, val, "'" #exp "' != '" #val "'", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

// clang-format on

extern int testing_fails;

// Listener of TestFail, like 'gtest::OnTestPartResult' event handler.
// Called in TestFail after a failed assertion.
typedef bool (*TestFailEventListener)(const char *expval, const char *val,
                                      const char *exp, const char *file,
                                      int line, const char *func);

// Prepare test engine (MSVC assertion setup, etc).
// listener - this function will be notified on each TestFail call.
void InitTestEngine(TestFailEventListener listener = nullptr);

// Release all test-engine resources.
// Prints or schedule a debug report if all test passed.
// Returns 0 if all tests passed or 1 otherwise.
// Memory leak report: FLATBUFFERS_MEMORY_LEAK_TRACKING && _MSC_VER && _DEBUG.
int CloseTestEngine(bool force_report = false);

// Write captured state to a log and terminate test run.
void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line, const char *func = nullptr);

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line, const char *func = nullptr);

// Workaround for `enum class` printing.
// There is an issue with the printing of enums with a fixed underlying type.
// These enums are generated by `flatc` if `--scoped-enums` is active.
// All modern compilers have problems with `std::stringstream&<<(T v)` if T is
// an enum with fixed type. For details see DR1601:
// http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1601
// https://stackoverflow.com/questions/34336024/ambiguous-overload-when-writing-an-enum-with-an-enum-base-but-only-with-clang

template<typename T, bool is_enum_type = flatbuffers::is_enum<T>::value>
struct underlying_of_scalar {
  static_assert(flatbuffers::is_scalar<T>::value, "invalid type T");
  typedef T type;
};

template<typename T> struct underlying_of_scalar<T, true> {
// clang-format off
  // There are old compilers without full C++11 support (see stl_emulation.h).
  #if defined(FLATBUFFERS_TEMPLATES_ALIASES)
  using type = typename std::underlying_type<T>::type;
  #else
  typedef int64_t type;
  #endif
  // clang-format on
};

template<typename T>
typename underlying_of_scalar<T>::type scalar_as_underlying(T v) {
  return static_cast<typename underlying_of_scalar<T>::type>(v);
}

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line,
            const char *func) {
  if (static_cast<U>(expval) != val) {
    TestFail(flatbuffers::NumToString(scalar_as_underlying(expval)).c_str(),
             flatbuffers::NumToString(scalar_as_underlying(val)).c_str(), exp,
             file, line, func);
  }
}

template<>
inline void TestEq<std::string, std::string>(std::string expval,
                                             std::string val, const char *exp,
                                             const char *file, int line,
                                             const char *func) {
  if (expval != val) {
    TestFail(expval.c_str(), val.c_str(), exp, file, line, func);
  }
}

template<typename T, typename U>
void TestNe(T expval, U val, const char *exp, const char *file, int line,
            const char *func) {
  if (static_cast<U>(expval) == val) {
    TestFail(flatbuffers::NumToString(scalar_as_underlying(expval)).c_str(),
             flatbuffers::NumToString(scalar_as_underlying(val)).c_str(), exp,
             file, line, func);
  }
}

template<>
inline void TestNe<std::string, std::string>(std::string expval,
                                             std::string val, const char *exp,
                                             const char *file, int line,
                                             const char *func) {
  if (expval == val) {
    TestFail(expval.c_str(), val.c_str(), exp, file, line, func);
  }
}

#endif  // !TEST_ASSERT_H
