#include <imgui.h>

#include "ChineseBitmapFont.hpp"
#include "Log.hpp"
#include "Scan.hpp"
#include "Patch.hpp"

namespace kanan {
    static ChineseBitmapFont* g_chineseBitmapFont{ nullptr };

    ChineseBitmapFont::ChineseBitmapFont()
        : m_isEnabled{ false },
        m_isFullwidth{ false }

    {
        g_chineseBitmapFont = this;
    }

    void ChineseBitmapFont::onUI() {
        if (ImGui::CollapsingHeader("Chinese Bitmap Font")) {
            ImGui::TextWrapped(
                "Enables chinese bitmap font"
                "NOTE: Bitmap Font patch have to be enabled."
            );
            ImGui::Spacing();
            ImGui::Checkbox("Enable Chinese Bitmap Font", &m_isEnabled);
            ImGui::Checkbox("Fullwidth characters", &m_isFullwidth);
        }
    }

    void ChineseBitmapFont::onConfigLoad(const Config& cfg) {
        m_isEnabled = cfg.get<bool>("ChineseBitmapFont.Enabled").value_or(false);
        m_isFullwidth = cfg.get<bool>("ChineseBitmapFont.Fullwidth").value_or(false);
        if(m_isEnabled)
            apply();
    }

    void ChineseBitmapFont::apply()
    {
        auto address = scan("client.exe", "83 F8 04 75 0F 57 56 E8 ? ? ? ? 83 C4 08 5F 5E 5B 5D C3");
        Patch p;
        if (address) {
            log("[ChineseBitmapFont] ping pleione::textintm_smallfont::GetCharacterDesc %p", *address);

            p.address = *address;
            p.bytes = { 0x83, 0xF8, 0x04, 0x90, 0x90 };
            patch(p);

        }
        else return;
        address = scan("client.exe", "83 C0 FE 83 F8 07 0F 87 ? ? ? ? FF 24 85 ? ? ? ? 68 ? ? ? ? 8D 4D E0 E8 ? ? ? ?");
        if (address) {
            log("[ChineseBitmapFont] ping font loading (1) %p", *address);

            p.address = *address;
            p.bytes = { 0x90, 0x90, 0x90 };
            patch(p);
        }
        else return;
        address = scan("client.exe", "83 E8 02 0F 84 ? ? ? ? 83 E8 02 0F 84 ? ? ? ? 83 E8 05 0F 84");
        if (address) {
            log("[ChineseBitmapFont] ping font loading (2) %p", *address);

            p.address = *address;
            p.bytes = { 0x83, 0xE8, 0x02, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
            patch(p);
        }
        else return;
        auto loadFonts_addr = scan("client.exe", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 5C 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? E8 ? ? ? ? 33 DB 89 45 B8 38 5D 08 0F 84");
        if (loadFonts_addr) {
            m_LoadFonts = reinterpret_cast<void(*)(int)>(*loadFonts_addr);
            int arg = 1;
            m_LoadFonts(arg);
            log("[ChineseBitmapFont] Got address of LoadFonts %p", m_LoadFonts);
        }
        else {
            log("[ChineseBitmapFont] Failed to get address of LoadFonts");
        }
        address = scan("client.exe", "55 8B EC 53 56 8B 75 08 33 DB 57 8B 7D 0C 56 89 5F 04 89 5F 08 89 5F 0C 89 5F 10 E8 ? ? ? ? 83 C4 04 84 C0 74 13");
        if (address)
        {
            log("[ChineseBitmapFont] Hooked GetCharacterDesc for fullwidth mod %p", *address);
            m_hookedGetCharacterDescHook = std::make_unique<FunctionHook>(*address, (uintptr_t)&ChineseBitmapFont::hookedGetCharacterDesc);
        }
    }

    int ChineseBitmapFont::hookedGetCharacterDesc(int charcode, int *a2, int a3, int a4)
    {
        auto orig = (decltype(hookedGetCharacterDesc)*)g_chineseBitmapFont->m_hookedGetCharacterDescHook->getOriginal();
        if (g_chineseBitmapFont->m_isFullwidth)
            if(charcode >= 0x21 && charcode <= 0x7F)
            {
                charcode = 0xFF00 ^ (charcode - 0x20);
            }
        return orig(charcode, a2, a3, a4);
    }

    void ChineseBitmapFont::onConfigSave(Config& cfg) {
        cfg.set<bool>("ChineseBitmapFont.Enabled", m_isEnabled);
        cfg.set<bool>("ChineseBitmapFont.Fullwidth", m_isFullwidth);
    }

}
