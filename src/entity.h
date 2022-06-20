#pragma once

#ifndef ENTITY_GUARD
#define ENTITY_GUARD

#include <vector>   // vector
#include <cstdint>  // uint8_t; int16_t; uint32_t

#include "vector.h"

#include "config.h"

class Entity
{
    public:
        Vector<float, 3>        get_pos()       const { return {pos[0] * 1.0f,
                                                                pos[1] * 1.0f,
                                                                pos[2] * 1.0f};     }
        Vector<std::uint8_t, 5> get_values()    const { return {desc_values1[0],
                                                                desc_values1[1],
                                                                desc_values2[0],
                                                                desc_values2[1],
                                                                desc_values2[2]};   }
        std::uint8_t            get_type()      const { return type;                }
        bool                    is_visible()    const { return visible;             }

    private:
        /* desc_values[*] hold values which describe said entity in more detail
         * (e. g. the brightness of a light, the pitch of playerstart, etc.). */
        Vector<std::int16_t, 3> pos;            // 0x0 -> 0x6 (6 bytes)
        Vector<std::uint8_t, 2> desc_values1;   // 0x6 -> 0x8 (2 bytes)
        std::uint8_t type;                      // 0x8 -> 0x9 (1 byte)
        Vector<std::uint8_t, 3> desc_values2;   // 0x9 -> 0xC (3 bytes)
        std::uint8_t visible;                   // 0xC -> 0xD (1 byte)
};

class DynamicEntity
{
    public:
        const Vector<float, 3> &get_pos()           const { return pos;             }
        std::uint32_t           get_spawn_time()    const { return spawn_time;      }
        std::uint32_t           get_despawn_delay() const { return despawn_delay;   }
        std::uint8_t            get_type()          const { return type;            }

        std::uint32_t get_time_left() const {
            return despawn_delay - (*Config::ticks - spawn_time);
        }
        
    private:
        std::uint8_t padding_0x0[4];    // 0x0  -> 0x4  (4 bytes)
        Vector<float, 3> pos;           // 0x4  -> 0x10 (6 bytes)
        std::uint8_t padding_0x10[124]; // 0x10 -> 0x8C (124 bytes)
        std::uint32_t spawn_time;       // 0x8C -> 0x90 (4 bytes)
        std::uint32_t despawn_delay;    // 0x90 -> 0x94 (4 bytes)
        std::uint8_t type;              // 0x94 -> 0x95 (1 byte)
};

class EntityList
{
    public:
        EntityList() = delete;

        static void repopulate();

        static void clear() { list.clear(); dyn_list.clear(); }

        static const std::vector<Entity *>          &get_list()     { return list;      }
        static const std::vector<DynamicEntity *>   &get_dyn_list() { return dyn_list;  }

    private:
        static std::vector<Entity *> list;
        static std::vector<DynamicEntity *> dyn_list;
};

#endif // ifndef ENTITY_GUARD