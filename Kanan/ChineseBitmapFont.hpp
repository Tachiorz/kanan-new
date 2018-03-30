#pragma once
#include <memory>

#include "Mod.hpp"
#include "FunctionHook.hpp"


namespace kanan {
    class ChineseBitmapFont : public Mod {
    public:
        ChineseBitmapFont();

        void onUI() override;

        void onConfigLoad(const Config& cfg) override;
        void onConfigSave(Config& cfg) override;
    private:
        bool m_isEnabled;
        bool m_isFullwidth;
        void(__cdecl *m_LoadFonts)(int);
        std::unique_ptr<FunctionHook> m_hookedGetCharacterDescHook;
        
        //char __cdecl pleione::textintm_smallfont::GetCharacterDesc(int a1, struct pleione::textintm_smallfont::character_desc *a2, int a3, int a4)
        static int __cdecl hookedGetCharacterDesc(int charcode, int *a2, int a3, int a4);
        

        void apply();
    };
}
