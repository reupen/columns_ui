#pragma once

namespace cui::artwork::legacy {

extern cfg_objList<pfc::string8> cfg_front_scripts;
extern cfg_objList<pfc::string8> cfg_back_scripts;
extern cfg_objList<pfc::string8> cfg_disc_scripts;
extern cfg_objList<pfc::string8> cfg_artist_scripts;

constexpr auto legacy_sources = {
    std::make_tuple(&cui::artwork::legacy::cfg_front_scripts, "Front cover"),
    std::make_tuple(&cui::artwork::legacy::cfg_back_scripts, "Back cover"),
    std::make_tuple(&cui::artwork::legacy::cfg_disc_scripts, "Disc cover"),
    std::make_tuple(&cui::artwork::legacy::cfg_artist_scripts, "Artist picture"),
};

bool any_legacy_sources();
void prompt_to_reconfigure();

} // namespace cui::artwork::legacy
