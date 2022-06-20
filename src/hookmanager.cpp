#include "hookmanager.h"

#include <string>

#include <Windows.h>

std::unordered_map<std::string, Hook> HookManager::hooks;

Hook &HookManager::hook(const std::string &name,
                        void *src, void *dst,
                        void *ptr_to_gateway_addr,
                        SIZE_T sz)
{
    const auto pos = hooks.find(name);
    if (hooks.cend() == pos) {
        return hooks.try_emplace(name, src, dst, ptr_to_gateway_addr, sz).first->second;
    }
    return pos->second;
}

Hook &HookManager::hook(const std::string &module_name,
                        const std::string &export_name,
                        void *dst,
                        void *ptr_to_gateway_addr,
                        SIZE_T sz)
{
    const auto pos = hooks.find(export_name);
    if (hooks.cend() == hooks.find(export_name)) {
        return hooks.try_emplace(export_name, module_name,
                                 export_name, dst,
                                 ptr_to_gateway_addr, sz).first->second;
    }
    return pos->second;
}

void HookManager::enable(const std::string &hook)
{
    const auto it = hooks.find(hook);
    if (hooks.cend() != it) {
        it->second.enable();
    }
}

void HookManager::disable(const std::string &hook)
{
    const auto it = hooks.find(hook);
    if (hooks.cend() != it) {
        it->second.disable();
    }
}