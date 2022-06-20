#include "hook.h"

#include <string>
#include <cstring>
#include <stdexcept>
#include <system_error>

#include <Windows.h>

#include "memory.h"

Hook::Hook(void *src, void *dst, void *ptr_to_gateway_addr, SIZE_T sz) :
                                                                       source_addr(src),
                                                                       destination_addr(dst),
                                                                       ptga(ptr_to_gateway_addr),
                                                                       byte_amt(sz)
{
    create_gateway();
}

Hook::Hook(const std::string &module_name, const std::string &export_name, void *dst,
           void *ptr_to_gateway_addr, SIZE_T sz) :
                                                 source_addr(Memory::get_export_address(module_name, export_name)),
                                                 destination_addr(dst),
                                                 ptga(ptr_to_gateway_addr),
                                                 byte_amt(sz)
{
    create_gateway();
}

void Hook::create_gateway()
{
    if (5 > byte_amt) {
        throw std::invalid_argument("create_gateway: invalid amount (<5) of bytes to overwrite specified.");
    }

    stolen_bytes = new BYTE[byte_amt];
    std::memcpy(stolen_bytes, source_addr, byte_amt);

    void *const gateway_addr = ::VirtualAlloc(nullptr, byte_amt, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!gateway_addr) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "create_gateway: could not allocate memory for gateway.");
    }
    
    std::memcpy(gateway_addr, source_addr, byte_amt);

    addr relative_offset = reinterpret_cast<addr>(source_addr) - reinterpret_cast<addr>(gateway_addr) - 5;
    *reinterpret_cast<addr *>(reinterpret_cast<addr>(gateway_addr) + byte_amt) = 0xE9;
    *reinterpret_cast<addr *>(reinterpret_cast<addr>(gateway_addr) + byte_amt + 1) = relative_offset;
    *reinterpret_cast<addr *>(ptga) = reinterpret_cast<addr>(gateway_addr);
}

void Hook::detour32() const
{
    DWORD old_protection = 0;
    if (!::VirtualProtect(source_addr, byte_amt, PAGE_EXECUTE_READWRITE, &old_protection)) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "detour32: could not acquire PAGE_EXECUTE_READWRITE rights.");
    }

    std::memset(source_addr, 0x90, byte_amt);

    addr relative_offset = reinterpret_cast<addr>(destination_addr) - reinterpret_cast<addr>(source_addr) - 5;
    *reinterpret_cast<addr *>(source_addr) = 0xE9;
    *reinterpret_cast<addr *>(reinterpret_cast<addr>(source_addr) + 1) = relative_offset;

    if (!::VirtualProtect(source_addr, byte_amt, old_protection, &old_protection)) {
        throw std::system_error(::GetLastError(), std::system_category(),
                                "detour32: could not reset protection.");
    }
}