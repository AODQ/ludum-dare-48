namespace ld {
    enum class ResearchType {
        Pickaxe = 0,
        Armor,
        Speed,
        Food,
        Cargo,
        Vision,
        Size,
    };

    struct ResearchInfoLookup {
        ld::ResearchType type;
        uint32_t cost;
        uint32_t maxLevel;
        const char* desc;
    };

    constexpr std::array<ld::ResearchInfoLookup, Idx(ld::ResearchType::Size)>
    researchInfoLookup = {{
        { .type = ld::ResearchType::Pickaxe, .cost = 100, .maxLevel = 10, .desc = "Increase mining power and pickaxe durability" },
        { .type = ld::ResearchType::Armor  , .cost = 300, .maxLevel = 1 , .desc = "Increase armor defense and durability"        },
        { .type = ld::ResearchType::Speed  , .cost = 200, .maxLevel = 3 , .desc = "Increase miners' walking speed"               },
        { .type = ld::ResearchType::Food   , .cost = 100, .maxLevel = 10, .desc = "Increase max food capacity and energy gain"   },
        { .type = ld::ResearchType::Cargo  , .cost = 100, .maxLevel = 10, .desc = "Increase miners' max cargo"                   },
        { .type = ld::ResearchType::Vision , .cost = 100, .maxLevel = 10, .desc = "Increase vision duration in fog"              },
    }};

    struct ResearchItem {
        ld::ResearchType type;
        uint32_t level = 0;
        std::string name;
    };

}
