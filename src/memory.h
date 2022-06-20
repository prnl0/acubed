#pragma once

#ifndef MEMORY_GUARD
#define MEMORY_GUARD

#include <string>
#include <cstdint>
#include <cstddef>
#include <vector>

#include <Windows.h>
#include <TlHelp32.h>

namespace Memory
{
    namespace Pattern
    {
        void *scan(const std::string &pattern,
                   const std::string &mask,
                   void *beg,
                   std::size_t len);

        void *scan_module(const std::string &pattern,
                          const std::string &mask,
                          const MODULEENTRY32 &me32);
        void *scan_module(const std::string &pattern,
                          const std::string &mask,
                          const std::wstring &module);
    } // namespace Pattern

    MODULEENTRY32 find_module(const std::wstring &name);
    void *get_export_address(const std::string &mod_name,
                             const std::string &exp_name);

    std::uintptr_t compute_addr_by_offsets(std::uintptr_t base_address,
                                           const std::vector<std::uintptr_t> &offsets);

    void patch_bytes(void *dst,
                     const void *src,
                     const std::size_t sz);
} // namespace Memory

#endif // ifndef MEMORY_GUARD