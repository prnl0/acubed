#include "entity.h"

#include <cstdint> // uint32_t

#include "config.h"
#include "constants.h"

std::vector<Entity *> EntityList::list;
std::vector<DynamicEntity *> EntityList::dyn_list;

void EntityList::repopulate()
{
    clear();

    auto count = *reinterpret_cast<std::uint32_t *>(Config::mod_base_address +
                                                    Offsets::Module::entity_count);
    const auto entity_list = *reinterpret_cast<Config::addr **>(Config::mod_base_address +
                             Offsets::Module::entity_list_pointer);

    for (std::uint32_t i = 0; count != i; ++i) {
        list.push_back(
            reinterpret_cast<Entity *>(reinterpret_cast<std::uintptr_t>(entity_list) + 0x14 * i)
        );
    }

    count = *reinterpret_cast<std::uint32_t *>(Config::mod_base_address +
                                               Offsets::Module::dynamic_entity_count);
    const auto dyn_entity_list = *reinterpret_cast<Config::addr ***>(Config::mod_base_address +
                                 Offsets::Module::dynamic_entity_list_pointer);

    for (std::uint32_t i = 0; count != i; ++i) {
        dyn_list.push_back(reinterpret_cast<DynamicEntity *>(dyn_entity_list[i]));
    }
}