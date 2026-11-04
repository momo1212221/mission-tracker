#include <cstdint>
#include <limits>
#include <string>
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
openrac::UiPage page;
std::string objective="No active objective";
std::string last_event="No recent progress";
std::uint32_t loads=std::numeric_limits<std::uint32_t>::max();
std::uint16_t previous=0;
bool open_requested=false;
}

void load(openrac::Mod& mod){
    if(mod.game()!="rac1") throw openrac::Error("Mission Tracker is for RAC1");
    g=&mod;

    page=mod.ui_page([&mod](const openrac::UiPage& p,const openrac::UiEvent&){
        const auto status=mod.play_status();
        openrac::UiContent c;
        c.title="Mission Tracker";
        c.items={
            openrac::UiItem::text(status.level>=0 ? "Planet: "+mod.level_name(status.level) : "Planet: -"),
            openrac::UiItem::text("Objective: "+objective),
            openrac::UiItem::text("Progress: "+last_event)
        };
        c.footer={"L1 + R1 + Left opens this page"};
        p.set(c);
    });

    mod.hook<openrac::rac1::Planet_Unlock>([](auto original,std::int32_t level){
        original(level);
        last_event="Destination unlocked: "+g->level_name(level);
    });
    mod.hook<openrac::rac1::Item_Give>([](auto original,std::int32_t id,std::int32_t equip){
        original(id,equip);
        last_event="Item acquired: "+std::to_string(id);
    });

    mod.on_pad([](int port,std::uint8_t* report,int size){
        if(port!=0 || report==nullptr || size<2) return;
        const auto now=qol::pad_buttons(report);
        const auto mask=static_cast<std::uint16_t>(OPENRAC_PAD_L1|OPENRAC_PAD_R1|OPENRAC_PAD_LEFT);
        if(qol::combo_pressed(now,previous,mask)) open_requested=true;
        if((now&mask)==mask) qol::consume_buttons(report,mask);
        previous=now;
    });

    mod.on_frame([]{
        const auto s=g->play_status();
        if(s.loads!=loads){
            loads=s.loads;
            objective=s.level>=0 ? qol::explore(g->level_name(s.level)) : "No active objective";
        }
        if(open_requested){
            open_requested=false;
            page.open();
        }
    });
}
OPENRAC_MOD_CPP("mission-tracker","0.2.0",load)
