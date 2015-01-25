#ifndef _CUI_ARTWORK_H_
#define _CUI_ARTWORK_H_

namespace artwork_panel
{

//extern cfg_string cfg_front, cfg_back, cfg_disc;//, cfg_icon;
extern cfg_uint cfg_fb2k_artwork_mode;
extern cfg_objList<pfc::string8> cfg_front_scripts, cfg_back_scripts, cfg_disc_scripts, cfg_artist_scripts;

album_art_extractor_instance_ptr g_get_album_art_extractor_instance(const char * path, abort_callback & p_abort);

enum fb2k_artwork_mode_t
{
	fb2k_artwork_disabled,
	fb2k_artwork_embedded,
	fb2k_artwork_embedded_and_external,
};

void g_on_edge_style_change();

}

#endif //_CUI_ARTWORK_H_