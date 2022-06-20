#pragma once

#ifndef HOOK_MANAGER_GUARD
#define HOOK_MANAGER_GUARD

#include <string>
#include <unordered_map>

#include <Windows.h>

#include "hook.h"

class HookManager
{
    public:
        HookManager() = delete;
        
        static Hook &hook(const std::string &name,
                          void *src,
                          void *dst,
                          void *ptr_to_gateway_addr,
                          SIZE_T sz);
        static Hook &hook(const std::string &module_name,
                          const std::string &export_name,
                          void *dst,
                          void *ptr_to_gateway_addr,
                          SIZE_T sz);

        static void enable(const std::string &hook);
        static void enable_all() { for (const auto &h : hooks) { h.second.enable(); } }

        static void disable(const std::string &hook);
        static void disable_all() { for (const auto &h : hooks) { h.second.disable(); } }

    private:
        static std::unordered_map<std::string, Hook> hooks;
};

#endif // HOOK_MANAGER_GUARD