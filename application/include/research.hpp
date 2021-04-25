namespace ld {
    enum class ResearchType {
        Pickaxe = 0,
        Armor,
        Food,
        Cargo,
        Vision,
        Speed,
        Size,
    };

    struct ResearchInfoLookup {
        ld::ResearchType type;
        uint32_t cost;
        const char* desc;
    };

    constexpr std::array<ld::ResearchInfoLookup, Idx(ld::ResearchType::Size)>
    researchInfoLookup = {{
        { .type = ld::ResearchType::Pickaxe, .cost = 25, .desc = "Increase mining power and pickaxe durability" },
        { .type = ld::ResearchType::Armor  , .cost = 25, .desc = "Increase armor defense and durability"        },
        { .type = ld::ResearchType::Food   , .cost = 25, .desc = "Increase max food capacity and energy gain"   },
        { .type = ld::ResearchType::Cargo  , .cost = 25, .desc = "Increase miners' max cargo"                   },
        { .type = ld::ResearchType::Vision , .cost = 25, .desc = "Increase vision duration in fog"              },
        { .type = ld::ResearchType::Speed  , .cost = 25, .desc = "Increase miners' walking speed"               },
    }};

    struct ResearchItem {
        ld::ResearchType type;
        uint32_t level = 0;
    };

}
