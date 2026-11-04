#include <cstdint>
#include <limits>
#include <string>
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
std::string temporary;
std::string base_line;
int timer=0;
std::uint32_t loads=std::numeric_limits<std::uint32_t>::max();
bool persistent=true;
}
void load(openrac::Mod& mod){
    if(mod.game()!="rac1" || !mod.has_notice()) throw openrac::Error("Mission Tracker requires RAC1 interface 1.9+");
    g=&mod; persistent=mod.setting<bool>("persistent_objective");
    mod.hook<openrac::rac1::Planet_Unlock>([](auto original,std::int32_t level){
        original(level); temporary="New destination: "+g->level_name(level); timer=240;
    });
    mod.hook<openrac::rac1::Item_Give>([](auto original,std::int32_t id,std::int32_t equip){
        original(id,equip); temporary="Progress updated - item "+std::to_string(id); timer=180;
    });
    mod.on_frame([]{
        const auto s=g->play_status();
        if(s.loads!=loads){
            loads=s.loads;
            base_line=s.level>=0 ? qol::explore(g->level_name(s.level)) : std::string();
            if(!base_line.empty()) g->play_notice(base_line);
        }
        if(!temporary.empty()){ g->play_notice(temporary); temporary.clear(); }
        else if(timer>0 && --timer==0) g->play_notice(persistent ? base_line : "");
    });
    mod.info("high-level objective/progress tracker");
}
OPENRAC_MOD_CPP("mission-tracker","0.1.0",load)
