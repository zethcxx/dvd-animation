include(CheckLinkerFlag)

# ------------------------------------------------------------
# FLAGS CONTAINERS
# ------------------------------------------------------------
set( CUSTOM_DEBUG_FLAGS        "" )
set( CUSTOM_RELEASE_FLAGS      "" )
set( CUSTOM_LINK_DEBUG_FLAGS   "" )
set( CUSTOM_LINK_RELEASE_FLAGS "" )

# ------------------------------------------------------------
# COMPILER DETECTION
# ------------------------------------------------------------
if ( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    set( IS_CLANG TRUE )
elseif ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    set( IS_GCC TRUE )
else()
    message( FATAL_ERROR "Unsupported compiler" )
endif()

# ------------------------------------------------------------
# ABI / OBJECT FORMAT
# ------------------------------------------------------------
if ( WIN32 )
    if ( CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC" )
        set( USE_COFF TRUE )
    else()
        set( USE_GNU_PE TRUE )
    endif()
else()
    set( USE_ELF TRUE )
endif()


# ------------------------------------------------------------
# COMMON FLAGS
# ------------------------------------------------------------
list( APPEND CUSTOM_DEBUG_FLAGS
    -O0
    -fno-omit-frame-pointer
    -fno-inline
    -fno-optimize-sibling-calls
    -fexceptions
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wsign-conversion
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wdouble-promotion
    -Wnull-dereference
    -Woverloaded-virtual
    -Wformat=2
    -fdiagnostics-color=always
)

list( APPEND CUSTOM_RELEASE_FLAGS
    -O2
    -fomit-frame-pointer
    -fdata-sections
    -ffunction-sections
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fstack-protector-strong
    -fdiagnostics-color=always
)


# ------------------------------------------------------------
# DEBUG INFO (ABI-DEPENDENT)
# ------------------------------------------------------------
if ( USE_ELF OR USE_GNU_PE )
    list( APPEND CUSTOM_DEBUG_FLAGS
        -g3
    )
elseif ( USE_COFF )
    list( APPEND CUSTOM_DEBUG_FLAGS
        -gcodeview
    )
endif()

# ------------------------------------------------------------
# ARCH TUNING
# ------------------------------------------------------------
if ( _NATIVE_BIN )
    list( APPEND CUSTOM_RELEASE_FLAGS
        -march=native
        -mtune=native
    )
else()
    list( APPEND CUSTOM_RELEASE_FLAGS
        -march=x86-64-v2
        -mtune=generic
    )
endif()

# ------------------------------------------------------------
# COMPILER-SPECIFIC
# ------------------------------------------------------------
if ( IS_CLANG )

    list( APPEND CUSTOM_DEBUG_FLAGS
        -Wcomma
        -Wextra-tokens
        -Wimplicit-fallthrough
        -fansi-escape-codes
    )

    list( APPEND CUSTOM_RELEASE_FLAGS
        -Oz
        -flto=thin
        -fansi-escape-codes
    )

elseif ( IS_GCC )

    list( APPEND CUSTOM_DEBUG_FLAGS
        -Wlogical-op
        -Wduplicated-cond
        -Wduplicated-branches
        -Wrestrict
        -Wuseless-cast
    )

    list( APPEND CUSTOM_RELEASE_FLAGS
        -Os
        -flto
        -fmerge-constants
    )

endif()

# ------------------------------------------------------------
# LINKER DETECTION (ELF ONLY)
# ------------------------------------------------------------
if ( USE_ELF AND IS_CLANG )

    check_linker_flag(CXX "-fuse-ld=lld" SUPPORT_LLD)
    check_linker_flag(CXX "-fuse-ld=gold" SUPPORT_GOLD)
    check_linker_flag(CXX "-fuse-ld=bfd"  SUPPORT_BFD)

    if ( SUPPORT_LLD )
        set( USED_LLD TRUE )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS -fuse-ld=lld)
    elseif ( SUPPORT_GOLD )
        set( USED_GOLD TRUE )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS -fuse-ld=gold)
    elseif ( SUPPORT_BFD )
        set( USED_BFD TRUE )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS -fuse-ld=bfd)
    endif()

endif()

# ------------------------------------------------------------
# ELF LINKER FLAGS
# ------------------------------------------------------------
if ( USE_ELF )

    list( APPEND CUSTOM_LINK_RELEASE_FLAGS
        -Wl,--gc-sections
        -Wl,--exclude-libs,ALL
        -Wl,--strip-all
    )

    list( APPEND CUSTOM_LINK_DEBUG_FLAGS
        -Wl,-z,relro
        -Wl,-z,now
        -Wl,-z,noexecstack
        -Wl,-z,defs
    )

    if ( USED_LLD )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS
            -Wl,--icf=all
            -Wl,--lto-O3
        )
    elseif ( USED_GOLD )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS
            -Wl,--icf=safe
            -Wl,--threads
        )
    elseif ( USED_BFD )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS
            -Wl,--as-needed
        )
    endif()

endif()

# ------------------------------------------------------------
# GNU PE (MinGW)
# ------------------------------------------------------------
if ( USE_GNU_PE )

    list( APPEND CUSTOM_LINK_RELEASE_FLAGS
        -Wl,--gc-sections
        -Wl,--strip-all
    )

endif()

# ------------------------------------------------------------
# COFF (Clang + MSVC ABI → lld-link)
# ------------------------------------------------------------
if ( USE_COFF )

    list( APPEND CUSTOM_LINK_DEBUG_FLAGS
        -Wl,/DEBUG
        -Wl,/INCREMENTAL
        -Wl,/OPT:NOREF
        -Wl,/OPT:NOICF
    )

    list( APPEND CUSTOM_LINK_RELEASE_FLAGS
        -Wl,/OPT:REF
        -Wl,/OPT:ICF
        -Wl,/INCREMENTAL:NO
        -Wl,/LTCG
        -Wl,/DYNAMICBASE
        -Wl,/NXCOMPAT
        -Wl,/HIGHENTROPYVA
    )

endif()

# ------------------------------------------------------------
# STATIC
# ------------------------------------------------------------
if ( _STATIC_BIN )

    if ( USE_ELF OR USE_GNU_PE )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS
            -static
            -static-libstdc++
            -static-libgcc
        )
    elseif ( USE_COFF )
        list( APPEND CUSTOM_LINK_RELEASE_FLAGS
            /MT
        )
    endif()

endif()
