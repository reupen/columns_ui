#include "stdafx.h"

toolbar_extension::button_image::button_image() : m_bm(0), m_mask_type(uie::MASK_NONE), m_bm_mask(0), m_mask_colour(0), m_icon(0)
{
	;
}

toolbar_extension::button_image::~button_image()
{
	if (m_bm)
		DeleteBitmap(m_bm);
	if (m_bm_mask)
		DeleteBitmap(m_bm_mask);
	if (m_icon)
		DestroyIcon(m_icon);
}
bool toolbar_extension::button_image::is_valid() { return m_bm != 0; }
void toolbar_extension::button_image::load(const button::custom_image & p_image)
{
	m_mask_type = p_image.m_mask_type;
	m_mask_colour = p_image.m_mask_colour;

	pfc::string8 fullPath;
	p_image.get_path(fullPath);

	if (!_stricmp(pfc::string_extension(fullPath), "bmp")) //Gdiplus vs 32bpp
		m_bm = (HBITMAP)uLoadImage(core_api::get_my_instance(), fullPath, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	else if (!_stricmp(pfc::string_extension(fullPath), "ico"))
		m_icon = (HICON)uLoadImage(core_api::get_my_instance(), fullPath, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE);
	else
	{
		m_bm = g_load_png_gdiplus(0, fullPath);
		//g_load_png_wic(0, p_image.m_path);
		//read_png(0, p_image.m_path);
	}
	if (m_bm)
	{
		switch (p_image.m_mask_type)
		{
		default:
			break;
		case ui_extension::MASK_COLOUR:
			break;
#if 0
		case ui_extension::MASK_BITMAP:
		{
			if (!_stricmp(pfc::string_extension(p_image.m_mask_path), "png"))
			{
				m_bm_mask = read_png(0, p_image.m_mask_path);
			}
			else
				m_bm_mask = (HBITMAP)LoadImage(core_api::get_my_instance(), pfc::stringcvt::string_os_from_utf8(p_image.m_mask_path), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE | LR_MONOCHROME);
			if (!m_bm_mask)
				console::printf("failed loading image \"%s\"", p_image.m_mask_path.get_ptr());
		}
		break;
#endif
		}
	}
	else if (!m_icon)
		console::printf("failed loading image \"%s\"", fullPath.get_ptr());
}
void toolbar_extension::button_image::load(const service_ptr_t<uie::button> & p_in, COLORREF colour_btnface, unsigned cx, unsigned cy)
{
	uie::button_v2::ptr inv2;
	if (p_in->service_query_t(inv2))
	{
		unsigned handle_type = 0;
		HANDLE image = inv2->get_item_bitmap(0, colour_btnface, cx, cy, handle_type);
		if (handle_type == uie::button_v2::handle_type_bitmap)
			m_bm = (HBITMAP)image;
		else if (handle_type == uie::button_v2::handle_type_icon)
			m_icon = (HICON)image;
	}
	else
		m_bm = p_in->get_item_bitmap(0, colour_btnface, m_mask_type, m_mask_colour, m_bm_mask);
}
unsigned toolbar_extension::button_image::add_to_imagelist(HIMAGELIST iml)
{
	unsigned rv = -1;
	if (m_icon)
	{
		rv = ImageList_ReplaceIcon(iml, -1, m_icon);
	}
	else if (m_bm)
	{
		switch (m_mask_type)
		{
		default:
			rv = ImageList_Add(iml, m_bm, 0);
			break;
		case ui_extension::MASK_COLOUR:
			rv = ImageList_AddMasked(iml, m_bm, m_mask_colour);
			break;
		case ui_extension::MASK_BITMAP:
		{
			rv = ImageList_Add(iml, m_bm, m_bm_mask);
		}
		break;
		}
	}
	return rv;
}
void toolbar_extension::button_image::get_size(SIZE & p_out)
{
	p_out.cx = 0;
	p_out.cy = 0;
	BITMAP bmi;
	memset(&bmi, 0, sizeof(BITMAP));
	if (m_icon)
	{
		ICONINFO ii;
		memset(&ii, 0, sizeof(ii));
		if (GetIconInfo(m_icon, &ii))
		{
			GetObject(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof(BITMAP), &bmi);
		}
	}
	else if (m_bm)
	{
		GetObject(m_bm, sizeof(BITMAP), &bmi);
	}
	p_out.cx = bmi.bmWidth;
	p_out.cy = bmi.bmHeight;
}

