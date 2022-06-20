#pragma once

#ifndef HOOK_GUARD
#define HOOK_GUARD

#include <cstdint>
#include <string>

#include <Windows.h>

#include "memory.h"

class Hook
{
    private:
        using addr = std::uintptr_t;

    public:
        explicit Hook(void *src, void *dst, void *ptr_to_gateway_addr, SIZE_T sz);
        explicit Hook(const std::string &module_name, const std::string &export_name, void *dst,
                      void *ptr_to_gateway_addr, SIZE_T sz);
        Hook(const Hook &) = delete;

        ~Hook() { delete[] stolen_bytes; }

        Hook &operator=(const Hook &) = delete;

        void enable() const { detour32(); }
        void disable() const { Memory::patch_bytes(source_addr, stolen_bytes, byte_amt); }

    private:
        void *source_addr = nullptr;
        const void *const destination_addr;
        void *const ptga; /* Pointer to gateway address. */

        const SIZE_T byte_amt;
        BYTE *stolen_bytes = nullptr;

        void create_gateway();
        void detour32() const;
};

#endif // HOOK_GUARD