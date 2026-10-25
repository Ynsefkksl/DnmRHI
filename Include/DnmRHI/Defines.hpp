#pragma once

#if defined(__GNUC__)
    #define DnmRHI_Force_Inline [[gnu::always_inline]] inline 
#elifdef __clang__
    #define DnmRHI_Force_Inline [[clang::always_inline]] inline 
#elifdef _MSC_VER
    #define DnmRHI_Force_Inline __forceinline
#else
    #define DnmRHI_Force_Inline inline
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define DnmRHI_Restrict __restrict
#elifdef _MSC_VER
    #define DnmRHI_Restrict __restrict__
#else
    #define DnmRHI_Restrict
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define DnmRHI_Func_Signature __PRETTY_FUNCTION__
#elifdef _MSC_VER
    #define DnmRHI_Func_Signature __FUNCSIG__
#else
    #define DnmRHI_Func_Signature
#endif
