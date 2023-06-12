/////////////////////////////////////////////////////////////////////////////////////
//
// Created by Ricardo Romero on 19/01/22.
// Copyright (c) 2022 Ricardo Romero.  All rights reserved.
//

#pragma once

#if (!defined(__cplusplus) || __cplusplus < 202002L)
#error "C++20 compiler or higher needed"
#endif /*__cplusplus*/

#ifndef CENTAUR_PROJECT
#define CENTAUR_PROJECT

#ifdef C_C_UNKNOWN
// This behavior might change in the future as more compilers are testes
#error "Centaur Unknown Compiler"
#endif /*C_C_UNKNOWN*/

// TODO: Replace macros to C_CLANG, C_GNU, C_GNU_CLANG or C_MSVC
#if defined(C_C_CLANG)
// These are cmake defined
#define C_CLANG 1
#else
// These are compiler defined
#if defined(__clang__)
#define C_CLANG 1
#endif /*__clang__*/
#endif

#if defined(C_C_GNU)
#define C_GNU 1
#else
#if (defined(__GNUC__) && !defined(C_CLANG))
#define C_GNU 1
#endif /*__GNUC__*/
#endif

#if defined(C_C_MSVC)
#define C_MSVC 1
#else
#if (defined(_MSC_VER) && !defined(C_CLANG) && !defined(C_GNU))
#define C_MSVC 1
#endif
#endif /*C_C_MSVC*/

#if defined(C_CLANG) || defined(C_GNU)
#define C_GNU_CLANG 1
#endif

#if !defined(C_CLANG) && !defined(C_GNU) && !defined(C_MSVC)
#error "Compiler could not be determinate"
#else
#define C_ALL_SUPPORTED 1
#endif

#define CENTAUR_VERSION_CODE(x, y, z) \
    (((x)*100000) + ((y)*100) + (z))

#define CENTAUR_STR(x)    CENTAUR_DO_STR(x)
#define CENTAUR_DO_STR(x) #x

#ifdef C_GNU_CLANG
#define CENTAUR_PRAGMA(x)      _Pragma(CENTAUR_STR(x))
#define CENTAUR_WARN_PRAGMA(x) CENTAUR_PRAGMA(GCC diagnostic x)
#define CENTAUR_WARN_OFF(x)    CENTAUR_WARN_PRAGMA(ignored x)
#define CENTAUR_WARN_PUSH()    CENTAUR_WARN_PRAGMA(push)
#define CENTAUR_WARN_POP()     CENTAUR_WARN_PRAGMA(pop)
#endif

// General Centaur namespace for libraries
#ifndef CENTAUR_NAMESPACE
#define CENTAUR_NAMESPACE cen
#endif /*CENTAUR_NAMESPACE*/

#ifndef BEGIN_CENTAUR_NAMESPACE
#define BEGIN_CENTAUR_NAMESPACE \
    namespace CENTAUR_NAMESPACE \
    {
#endif /*BEGIN_CENTAUR_NAMESPACE*/

#ifndef END_CENTAUR_NAMESPACE
#define END_CENTAUR_NAMESPACE }
#endif /*END_CENTAUR_NAMESPACE*/

#ifndef C_NODISCARD
#if defined(__clang__) || defined(__GNU__) || defined(MSVC)
#define C_NODISCARD [[nodiscard]]
#else
#define C_NODISCARD
#endif /* defined ...*/
#endif /*C_NODISCARD*/

#ifndef C_UNUSED
#if defined(__clang__) || defined(__GNU__) || defined(MSVC)
#define C_UNUSED [[maybe_unused]]
#else
#define C_UNUSED
#endif /* defined ...*/
#endif /*C_UNUSED*/

#ifndef C_NORETURN
#if defined(__clang__) || defined(__GNU__) || defined(MSVC)
#define C_NORETURN [[noreturn]]
#else
#define C_NORETURN
#endif /* defined ...*/
#endif /*C_NORETURN*/

#ifndef C_FALLTHROUGH
#if defined(__clang__) || defined(__GNU__) || defined(MSVC)
#define C_FALLTHROUGH [[fallthrough]]
#else
#define C_FALLTHROUGH
#endif /* defined ...*/
#endif /*C_FALLTHROUGH*/

namespace CENTAUR_NAMESPACE
{
    constexpr char CentaurVersionString[]         = "0.1.0";
    constexpr char CentaurProtocolVersionString[] = "0.1.0";
} // namespace CENTAUR_NAMESPACE

#endif // CENTAUR_PROJECT
