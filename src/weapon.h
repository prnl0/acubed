#pragma once

#ifndef WEAPON_GUARD
#define WEAPON_GUARD

#include <string>
#include <cstdint>

class WeaponInfo
{
    public:
        void set_spread(std::int16_t val)       { spread = val;             }
        void set_knockback(std::int16_t val)    { knockback = val;          }
        void set_recoil(std::int16_t val)       { recoil = val;             }
        void set_auto_attack(bool state)        { can_auto_attack = state;  }

        std::int16_t &get_spread()      { return spread;    }
        std::int16_t &get_knockback()   { return knockback; }
        std::int16_t &get_recoil()      { return recoil;    }
        
        std::string     get_name()              const   { return name;              }
        std::int16_t    get_reload_delay()      const   { return reload_delay;      }
        std::int16_t    get_spread()            const   { return spread;            }
        std::int16_t    get_knockback()         const   { return knockback;         }
        std::int16_t    get_recoil()            const   { return recoil;            }
        std::uint8_t    get_can_auto_attack()   const   { return can_auto_attack;   }

    private:
        char name[16];                      // 0x0   -> 0x10  (16 bytes)
        std::uint8_t padding_0x10[248];     // 0x10  -> 0x108 (248 bytes)
        std::int16_t reload_delay;          // 0x108 -> 0x10A (2 bytes)
        std::int16_t animation_duration;    // 0x10A -> 0x10C (2 bytes)
        std::uint8_t padding_0x10C[8];      // 0x10C -> 0x114 (8 bytes)
        std::int16_t spread;                // 0x114 -> 0x116 (2 bytes)
        std::int16_t knockback;             // 0x116 -> 0x118 (2 bytes)
        std::uint8_t max_clip;              // 0x118 -> 0x119 (1 byte)
        std::uint8_t padding_0x119[9];      // 0x119 -> 0x122 (9 bytes)
        std::int16_t recoil;                // 0x122 -> 0x124 (2 byte)
        std::uint8_t padding_0x123[4];      // 0x124 -> 0x128 (4 bytes)
        std::uint8_t can_auto_attack;       // 0x128 -> 0x129 (1 byte)
};

class Weapon
{
    public:
        void set_spread(std::int16_t val)       { wi->set_spread(val);          }
        void set_knockback(std::int16_t val)    { wi->set_knockback(val);       }
        void set_recoil(std::int16_t val)       { wi->set_recoil(val);          }
        void set_auto_attack(bool state)        { wi->set_auto_attack(state);   }

        static void toggle_consec_shots(bool state);
        static void toggle_auto_attack_all(bool state);
        static void reset_rsk(); // Recoils, spreads, knockbacks

        std::int16_t &get_spread()      { return wi->get_spread();     }
        std::int16_t &get_knockback()   { return wi->get_knockback();  }
        std::int16_t &get_recoil()      { return wi->get_recoil();     }
        
        std::string     get_name()              const { return wi->get_name();              }
        std::int32_t    get_id()                const { return id;                          }
        std::int32_t    get_mag()               const { return *magazine;                   }
        std::int32_t    get_clip()              const { return *clip;                       }
        std::int32_t    get_attack_delay()      const { return *attack_delay;               }
        std::int16_t    get_reload_delay()      const { return wi->get_reload_delay();      }
        std::uint32_t   get_reload_start()      const { return reload_start;                }
        std::int16_t    get_spread()            const { return wi->get_spread();            }
        std::int16_t    get_knockback()         const { return wi->get_knockback();         }
        std::int16_t    get_recoil()            const { return wi->get_recoil();            }
        std::uint8_t    get_can_auto_attack()   const { return wi->get_can_auto_attack();   }
    
    private:
        std::uint8_t padding_0x0[4];    // 0x0  -> 0x4  (4 bytes)
        std::int32_t id;                // 0x4  -> 0x8  (4 bytes)
        std::uint8_t padding_0x8[4];    // 0x8  -> 0xC  (4 bytes)
        WeaponInfo *wi;                 // 0xC  -> 0x10 (4 bytes)
        std::uint32_t *magazine;        // 0x10 -> 0x14 (4 bytes)
        std::uint32_t *clip;            // 0x14 -> 0x18 (4 bytes)
        std::uint32_t *attack_delay;    // 0x18 -> 0x1C (4 bytes)
        std::uint8_t padding_0x1C[4];   // 0x1C -> 0x20 (4 bytes)
        std::uint32_t reload_start;     // 0x20 -> 0x24 (4 bytes)
};

#endif // WEAPON_GUARD