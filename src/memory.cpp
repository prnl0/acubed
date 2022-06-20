#include "memory.h"

#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <system_error>

#include <Windows.h>
#include <TlHelp32.h>

void *Memory::Pattern::scan(const std::string &pattern,
                            const std::string &mask,
                            void *beg,
                            std::size_t len)
{
    // TODO: do some check to make sure pattern and mask contain the same
    //       amount of bytes (take into consideration the fact that if supplied
    //       with byte 00, it is interpreted as a null ('\0') character,
    //       effectively altering the actual size of the byte array).
    // assert(check(...));

    for (char *curr = static_cast<char *>(beg), *end = curr + len; end != curr; ++curr) {
        bool found = true;
        for (std::string::size_type i = 0; pattern.size() != i; ++i) {
            if ('?' != mask[i] && curr[i] != pattern[i]) {
                found = false;
                break;
            }
        }

        if (found) {
            return static_cast<void*>(curr);
        }
    }

    return nullptr;
}

void *Memory::Pattern::scan_module(const std::string &pattern,
                                   const std::string &mask,
                                   const MODULEENTRY32 &me32)
{
    // TODO: do some check to make sure pattern and mask contain the same
    //       amount of bytes (take into consideration the fact that if supplied
    //       with byte 00, it is interpreted as a null ('\0') character,
    //       effectively altering the actual size of the byte array).
    // assert(check(...));

    if (!me32.modBaseAddr) {
        return nullptr;
    }

    MEMORY_BASIC_INFORMATION mbi;
    for (BYTE *curr = me32.modBaseAddr, *end = curr + me32.modBaseSize;
         end > curr; curr += mbi.RegionSize) {
        if (!::VirtualQuery(curr, &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
            throw std::system_error(::GetLastError(), std::system_category(),
                                    "scan_module: VirtualQuery failed.");
        }

        // comment
        if (!(mbi.State & MEM_COMMIT)) {
            continue;
        }

        // comment
        DWORD old_protect = 0;
        if (mbi.Protect & PAGE_NOACCESS &&
            !::VirtualProtect(curr, sizeof(MEMORY_BASIC_INFORMATION), PAGE_EXECUTE_READ, &old_protect)) {
            throw std::system_error(::GetLastError(), std::system_category(),
                                    "scan_module: could not acquire PAGE_EXECUTE_READ rights.");
        }

        void *const address = scan(pattern, mask, curr, mbi.RegionSize);

        if (mbi.Protect & PAGE_NOACCESS &&
            !::VirtualProtect(curr, sizeof(MEMORY_BASIC_INFORMATION), old_protect, &old_protect)) {
            throw std::system_error(::GetLastError(), std::system_category(),
                                    "scan_module: could not reset protection.");
        }

        if (address) {
            return address;
        }
    }

    return nullptr;
}

void *Memory::Pattern::scan_module(const std::string &pattern,
                                   const std::string &mask,
                                   const std::wstring &module)
{
    // TODO: do some check to make sure pattern and mask contain the same
    //       amount of bytes (take into consideration the fact that if supplied
    //       with byte 00, it is interpreted as a null ('\0') character,
    //       effectively altering the actual size of the byte array).
    // assert(check(...));

    return scan_module(pattern, mask, find_module(module));
}

MODULEENTRY32 Memory::find_module(const std::wstring &name)
{
    HANDLE snap = nullptr;
    while (INVALID_HANDLE_VALUE == (snap = ::CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE, ::GetCurrentProcessId()))) {}

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);
    if (Module32First(snap, &me32)) {
        do {
            if (!_wcsicmp(me32.szModule, name.c_str())) {
                return me32;
            }
        } while (Module32Next(snap, &me32));
    }

    return MODULEENTRY32{};
}

void *Memory::get_export_address(const std::string &mod_name,
                                 const std::string &exp_name)
{
    HMODULE hmod = ::GetModuleHandleA(mod_name.c_str());
    if (!hmod) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "get_export_address: unable to retrieve module handle of \"" + mod_name + "\".");
    }

    void *addr = ::GetProcAddress(hmod, exp_name.c_str());
    if (!addr) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "get_export_address: unable to retrieve address of \"" + exp_name + "\".");
    }

    return addr;
}

std::uintptr_t Memory::compute_addr_by_offsets(std::uintptr_t base_addr,
                                               const std::vector<std::uintptr_t> &offsets)
{
    std::uintptr_t &curr_addr = base_addr;
    for (const unsigned &offset : offsets) {
        curr_addr = *reinterpret_cast<std::uintptr_t*>(curr_addr);
        curr_addr += offset;
    }
    return curr_addr;
}

void Memory::patch_bytes(void *dst,
                         const void *src,
                         const std::size_t sz)
{
    DWORD old_protect = 0;
    if (!::VirtualProtect(dst, sz, PAGE_EXECUTE_READWRITE, &old_protect)) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "patch_bytes: could not acquire PAGE_EXECUTE_READWRITE rights.");
    }

    std::memcpy(dst, src, sz);

    if (!::VirtualProtect(dst, sz, old_protect, &old_protect)) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "patch_bytes: could not reset protection.");
    }
}