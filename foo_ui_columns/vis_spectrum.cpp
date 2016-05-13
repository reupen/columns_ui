#include "stdafx.h"
#include "vis_gen_host.h"
#include "main_window.h"

#if 1
class spectrum_vis_callback;
class spectrum_extension;

extern cfg_int cfg_vis_edge;

enum 
{
	MODE_STANDARD,
	MODE_BARS,
};

enum 
{
	scale_linear,
	scale_logarithmic,
};

// {DFA4E08C-325F-4b32-91EB-CD9FD5D0AD14}
const GUID g_guid_scale = 
{ 0xdfa4e08c, 0x325f, 0x4b32, { 0x91, 0xeb, 0xcd, 0x9f, 0xd5, 0xd0, 0xad, 0x14 } };

// {3323C764-875A-4464-AC8E-BB130E215A4C}
const GUID g_guid_vertical_scale = 
{ 0x3323c764, 0x875a, 0x4464, { 0xac, 0x8e, 0xbb, 0x13, 0xe, 0x21, 0x5a, 0x4c } };

cfg_int cfg_vis_mode(create_guid(0x3341d306,0xf8b6,0x6c60,0xbd,0x7e,0xe4,0xc5,0xab,0x51,0xf3,0xdd),MODE_BARS);
cfg_int cfg_scale(g_guid_scale, scale_logarithmic);
cfg_int cfg_vertical_scale(g_guid_vertical_scale, scale_logarithmic);

class spectrum_extension : public ui_extension::visualisation, public play_callback
{
	ui_extension::visualisation_host_ptr p_host;

protected:
public:

	bool b_active;

	HBRUSH br_foreground, br_background;

	COLORREF cr_fore;
	COLORREF cr_back;
	unsigned mode;

	unsigned short m_bar_width, m_bar_gap;

	t_size m_scale, m_vertical_scale;

	static pfc::ptr_list_t<spectrum_extension> list_vis;

	void make_bitmap(HDC hdc, const RECT * rc_area);
	//void flush_bitmap();
	void flush_brushes();

	spectrum_extension();

	~spectrum_extension();

	static const GUID extension_guid;

	const GUID & get_extension_guid() const override
	{
		return extension_guid;
	}

	void get_name(pfc::string_base & out)const override;


	void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort) override;
	void enable( const ui_extension::visualisation_host_ptr & p_host) override;
	void paint_background(HDC dc, const RECT * rc_client) override;
	void disable() override; 

	inline void clear()
	{
		refresh(nullptr);
	}

	bool have_config_popup()const override{return true;}

	bool show_config_popup(HWND wnd_parent) override;

	void get_config(stream_writer * data, abort_callback & p_abort) const override;

	static pfc::ptr_list_t<spectrum_extension> g_visualisations;

	static void g_register_stream(spectrum_extension * p_ext)
	{
		if (!g_visualisations.have_item(p_ext))
		{
			//console::printf("registering %x",p_ext);
			if (g_visualisations.add_item(p_ext) == 0)
			{
				//static_api_ptr_t<visualisation_manager>()->create_stream(g_stream, NULL);
				g_create_timer();
			}
		}
	}

	static void g_deregister_stream(spectrum_extension * p_ext, bool b_paused = false)
	{
		//console::printf("deregistering %x",p_ext);
		g_visualisations.remove_item(p_ext);
		if (!g_visualisations.get_count())
		{
			g_destroy_timer();
			//g_stream.release();
		}
		if (!b_paused)
		{
			if (p_ext->b_active)
				p_ext->clear();
			p_ext->flush_brushes();
		}
	}

	static bool g_is_stream_active(spectrum_extension * p_ext)
	{
		return g_visualisations.have_item(p_ext);
	}

	friend class spectrum_vis_callback;
	friend class spec_param;

private:
	static UINT_PTR g_timer_refcount;
	static UINT_PTR g_timer;
	static service_ptr_t<visualisation_stream> g_stream;

	static void CALLBACK g_timer_proc(HWND wnd, UINT msg,UINT_PTR id_event,DWORD time);
	static void g_refresh_all();

	void refresh(const audio_chunk * p_chunk);
	static void g_create_timer()
	{
		if (!g_timer)
		{
			g_timer = SetTimer(nullptr, NULL, 25, g_timer_proc);
			g_refresh_all();
		}
	}

	static void g_destroy_timer()
	{
		if (g_timer)
		{
			KillTimer(nullptr, g_timer);
			g_timer = NULL;
		}
	}

	void FB2KAPI on_playback_starting(play_control::t_track_command p_command,bool p_paused) override
	{
	};
	void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track) override
	{
		g_register_stream(this);
	};
	void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason) override
	{
		g_deregister_stream(this, p_reason == play_control::stop_reason_shutting_down);
	};
	void FB2KAPI on_playback_seek(double p_time) override{};
	void FB2KAPI on_playback_pause(bool p_state) override
	{
		if (p_state)
			g_deregister_stream(this, true);
		else
			g_register_stream(this);
	};
	void FB2KAPI on_playback_edited(metadb_handle_ptr p_track) override {};
	void FB2KAPI on_playback_dynamic_info(const file_info & p_info) override {};
	void FB2KAPI on_playback_dynamic_info_track(const file_info & p_info) override {};
	void FB2KAPI on_playback_time(double p_time) override {};
	void FB2KAPI on_volume_change(float p_new_val) override {};
};

UINT_PTR spectrum_extension::g_timer = NULL;
UINT_PTR spectrum_extension::g_timer_refcount = NULL;
service_ptr_t<visualisation_stream> spectrum_extension::g_stream;

pfc::ptr_list_t<spectrum_extension> spectrum_extension::list_vis;
pfc::ptr_list_t<spectrum_extension> spectrum_extension::g_visualisations;


spectrum_extension::spectrum_extension()
	: b_active(false), br_foreground(nullptr), br_background(nullptr), cr_fore(cfg_vis2), cr_back(cfg_vis), 
	mode(cfg_vis_mode), m_bar_width(3), m_bar_gap(1), m_scale(cfg_scale),
	m_vertical_scale(cfg_vertical_scale)
{
};

spectrum_extension::~spectrum_extension()
= default;

void spectrum_extension::flush_brushes()
{
	if (br_background)
	{
		DeleteObject(br_background);
		br_background=nullptr;
	}
	if (br_foreground)
	{
		DeleteObject(br_foreground); 
		br_foreground=nullptr; 
	}
}

void spectrum_extension::paint_background(HDC dc, const RECT * rc_client)
{
	if (!br_background)
		br_background = CreateSolidBrush(cr_back);

	FillRect(dc,rc_client,br_background);
}

void spectrum_extension::enable( const ui_extension::visualisation_host_ptr & p_vis_host)
{
	p_host = p_vis_host;
	b_active = true;

	m_bar_width = uih::ScaleDpiValue(3);
	m_bar_gap = uih::ScaleDpiValue(1);
	
	if (list_vis.add_item(this) == 0)
	{
		static_api_ptr_t<visualisation_manager>()->create_stream(g_stream, visualisation_manager::KStreamFlagNewFFT);
		visualisation_stream_v2::ptr p_stream_v2;
		if (g_stream->service_query_t(p_stream_v2))
			p_stream_v2->set_channel_mode(visualisation_stream_v2::channel_mode_mono);
	}

	static_api_ptr_t<play_callback_manager>()->register_callback(this, play_callback::flag_on_playback_new_track|play_callback::flag_on_playback_stop|play_callback::flag_on_playback_pause, false);
	if (static_api_ptr_t<play_control>()->is_playing())
		g_register_stream(this);
}


void spectrum_extension::disable()
{
	b_active = false;

	list_vis.remove_item(this);

	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
	if (static_api_ptr_t<play_control>()->is_playing())
		g_deregister_stream(this);

	if (!list_vis.get_count())
		g_stream.release();

	p_host.release();
}

class spec_param
{
public:
	modal_dialog_scope m_scope;
	COLORREF cr_fore;
	COLORREF cr_back;
	unsigned mode;
	t_size m_scale, m_vertical_scale;
	spectrum_extension * ptr;
	HBRUSH br_fore;
	HBRUSH br_back;
	unsigned frame;
	bool b_show_frame;
	void flush_back()
	{
		if (br_back) {
			DeleteObject(br_back);
			br_back = nullptr;
		}
	}
	void flush_fore()
	{
		if (br_fore){
			DeleteObject(br_fore);
			br_fore = nullptr;
		}
	}
	spec_param(COLORREF fore, COLORREF back, unsigned p_mode, t_size scale, t_size vertical_scale, spectrum_extension * p_spec, bool p_show_frame = false, unsigned p_frame = 0)
		: cr_fore(fore), cr_back(back), mode(p_mode), m_scale(scale), m_vertical_scale(vertical_scale), ptr(p_spec), br_fore(nullptr), br_back(nullptr), frame(p_frame), b_show_frame(p_show_frame)
	{};
	~spec_param() {
		flush_back();
		flush_fore();
	}
};

static BOOL CALLBACK SpectrumPopupProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(wnd,DWLP_USER,lp);
		{
			spec_param * ptr = reinterpret_cast<spec_param*>(lp);
			ptr->m_scope.initialize(FindOwningPopup(wnd));
			SendDlgItemMessage(wnd, IDC_BARS, BM_SETCHECK, ptr->ptr->mode == MODE_BARS, 0);
			HWND wnd_combo = GetDlgItem(wnd, IDC_FRAME_COMBO);
			EnableWindow(wnd_combo, ptr->b_show_frame);
			if (ptr->b_show_frame)
			{
				ComboBox_AddString(wnd_combo, _T("None"));
				ComboBox_AddString(wnd_combo, _T("Sunken"));
				ComboBox_AddString(wnd_combo, _T("Grey"));
				ComboBox_SetCurSel(wnd_combo, ptr->frame);
			}
			wnd_combo = GetDlgItem(wnd, IDC_SCALE);
			ComboBox_AddString(wnd_combo, _T("Linear"));
			ComboBox_AddString(wnd_combo, _T("Logarithmic"));
			ComboBox_SetCurSel(wnd_combo, ptr->m_scale);

			wnd_combo = GetDlgItem(wnd, IDC_VERTICAL_SCALE);
			ComboBox_AddString(wnd_combo, _T("Linear"));
			ComboBox_AddString(wnd_combo, _T("Logarithmic"));
			ComboBox_SetCurSel(wnd_combo, ptr->m_vertical_scale);
		}
		return TRUE;
	case WM_ERASEBKGND:
		SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
		return TRUE;
	case WM_PAINT:
		uih::HandleModernBackgroundPaint(wnd, GetDlgItem(wnd, IDOK));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		{
			spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
			if (GetDlgItem(wnd, IDC_PATCH_FORE) == (HWND)lp) {
				HDC dc =(HDC)wp;
				if (!ptr->br_fore) 
				{
					ptr->br_fore = CreateSolidBrush(ptr->cr_fore);
				}
				return (BOOL)ptr->br_fore;
			} 
			else if (GetDlgItem(wnd, IDC_PATCH_BACK) == (HWND)lp) 
			{
				HDC dc =(HDC)wp;
				if (!ptr->br_back) 
				{
					ptr->br_back = CreateSolidBrush(ptr->cr_back);
				}
				return (BOOL)ptr->br_back;
			}
			else
			return (BOOL)GetSysColorBrush(COLOR_3DHIGHLIGHT);
		}
		break;
	case WM_COMMAND:
		switch(wp)
		{
		case IDCANCEL:
			EndDialog(wnd,0);
			return TRUE;
		case IDC_CHANGE_BACK:
			{
				spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
				COLORREF COLOR = ptr->cr_back;
				COLORREF COLORS[16] = {get_default_colour(colours::COLOUR_BACK),GetSysColor(COLOR_3DFACE),0,0,0,0,0,0,0,0,0,0,0,0,0,0};
				if (uChooseColor(&COLOR, wnd, &COLORS[0]))
				{
					ptr->cr_back = COLOR;
					ptr->flush_back();
					RedrawWindow(GetDlgItem(wnd, IDC_PATCH_BACK), nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW);
				}
			}
			break;
		case IDC_CHANGE_FORE:
			{
				spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
				COLORREF COLOR = ptr->cr_fore;
				COLORREF COLORS[16] = {get_default_colour(colours::COLOUR_TEXT),GetSysColor(COLOR_3DSHADOW),0,0,0,0,0,0,0,0,0,0,0,0,0,0};
				if (uChooseColor(&COLOR, wnd, &COLORS[0]))
				{
					ptr->cr_fore = COLOR;
					ptr->flush_fore();
					RedrawWindow(GetDlgItem(wnd, IDC_PATCH_FORE), nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW);
				}
			}
			break;
		case IDC_BARS:
			{
				spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
				ptr->mode = (SendMessage((HWND)lp, BM_GETCHECK, 0, 0) != TRUE ? MODE_STANDARD : MODE_BARS);
			}
			break;
		case IDC_FRAME_COMBO|(CBN_SELCHANGE<<16):
			{
				spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
				ptr->frame = ComboBox_GetCurSel(HWND(lp));
			}
			break;
		case IDC_SCALE|(CBN_SELCHANGE<<16):
			{
				spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
				ptr->m_scale = ComboBox_GetCurSel(HWND(lp));
			}
			break;
		case IDC_VERTICAL_SCALE|(CBN_SELCHANGE<<16):
			{
				spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
				ptr->m_vertical_scale = ComboBox_GetCurSel(HWND(lp));
			}
			break;
		case IDOK:
			{
				spec_param * ptr = reinterpret_cast<spec_param*>(GetWindowLongPtr(wnd,DWLP_USER));
				EndDialog(wnd,1);
			}
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
}


bool spectrum_extension::show_config_popup(HWND wnd_parent)
{
	spec_param param(cr_fore, cr_back, mode, m_scale, m_vertical_scale, this);
	bool rv = !!uDialogBox(IDD_POPUP_SPECTRUM_NEW, wnd_parent, SpectrumPopupProc, (LPARAM)(&param));
	if (rv)
	{
		cr_fore = param.cr_fore;
		cfg_vis2 = param.cr_fore;
		cr_back = param.cr_back;
		cfg_vis = param.cr_back;
		mode = param.mode;
		cfg_vis_mode = param.mode;
		m_scale = param.m_scale;
		cfg_scale = param.m_scale;
		m_vertical_scale = param.m_vertical_scale;
		cfg_vertical_scale = param.m_vertical_scale;
		if (b_active)
		{
			flush_brushes();
			clear();
		}
	}
	return rv;
}

/*
void spectrum_extension::flush_bitmap()
{
	ui_extension::visualisation_host::vis_paint_struct ps;
	p_host->begin_paint(ps);
	paint_background(ps.dc, &ps.rc_client);
	p_host->end_paint(ps);
}
*/

void CALLBACK spectrum_extension::g_timer_proc(HWND wnd, UINT msg,UINT_PTR id_event,DWORD time)
{
	g_refresh_all();
}

void g_scale_value(t_size source_count, t_size index, t_size dest_count, t_size & source_start, t_size & source_end, bool b_log)
{
	double start, end;
	if (b_log)
	{
		static constexpr auto power = 500;
		static const double exp0 = pow(power, 0);
		static const double exp1 = pow(power, 1);
		start = (double)source_count * (pow(power, double(index) / (double)dest_count) - exp0) / (exp1 - exp0);
		end = (double)source_count *(pow(power, double(index + 1) / (double)dest_count) - exp0) / (exp1 - exp0);
	}
	else
	{
		start = (double)source_count*((double)(index)/(double)dest_count);
		end = (double)source_count*((double)(index+1)/(double)dest_count);
	}
	source_start = pfc::rint32(start);
	source_end = pfc::rint32(end);
	if (source_end > source_start)
		--source_end;
}

t_size g_scale_value_single(double val, t_size count, bool b_log)
{
	double val_trans;
	if (b_log)
	{
		constexpr auto minimum_value = -4;
		double log_val = val > 0 ? log10(val) : minimum_value;
		if (log_val < minimum_value)
			log_val = minimum_value;
		val_trans = count * (log_val + -minimum_value) / -minimum_value;
	}
	else
	{
		double start = (double)count*val;
		val_trans = start;
	}
	t_size ret = pfc::rint32(val_trans);
	ret = max(min(ret, count), 0);
	return ret;
}

void spectrum_extension::refresh(const audio_chunk * p_chunk)
{
	ui_extension::visualisation_host::painter_ptr ps;
	p_host->create_painter(ps);

	HDC dc = ps->get_device_context();
	const RECT * rc_client = ps->get_area();
	{
		paint_background(dc, rc_client);

		if (g_is_stream_active(this) && p_chunk)
		{
			if (!br_foreground)
				br_foreground = CreateSolidBrush(cr_fore);


			if (mode == MODE_BARS)
			{
				unsigned totalbars = rc_client->right / m_bar_width;
				if (totalbars)
				{
					const audio_sample * p_data = p_chunk->get_data();
					t_size sample_count = p_chunk->get_sample_count();
					t_size channel_count = p_chunk->get_channels();
					t_size i;
					for(i=0;i<totalbars;i++)
					{
						double val = 0;
						t_size starti, endi, j;
						g_scale_value(sample_count, i, totalbars, starti, endi, m_scale == scale_logarithmic);
						for (j=starti;j<=endi; j++)
						{
							if (j < sample_count)
							{
								double sample_val = 0;
								t_size k;
								for (k=0; k<channel_count; k++)
									sample_val += p_data[j*channel_count + k];
								sample_val *= 1.0 / channel_count;
								val = max(val, sample_val);
							}

						}

						RECT r;
						r.left = 1 + i*m_bar_width;
						r.right = r.left + m_bar_width - m_bar_gap;
						r.bottom = rc_client->bottom ? rc_client->bottom-1 : 0;
						r.top = rc_client->bottom - g_scale_value_single(val, (rc_client->bottom + 1)/2 , m_vertical_scale == scale_logarithmic)*2;
						if (r.bottom>r.top)
							FillRect(dc,&r,br_foreground);
					}
					int j;
					for (j=rc_client->bottom; j>rc_client->top; j-=2)
					{
						RECT rc = {0, j-1, rc_client->right, j};
						FillRect(dc,&rc,br_background);
					}
				}
			}
			else
			{
				const audio_sample * p_data = p_chunk->get_data();
				t_size sample_count = p_chunk->get_sample_count();
				t_size channel_count = p_chunk->get_channels();
				t_size i;
				for(i=0;i<(t_size)rc_client->right;i++)
				{
					double val = 0;
					t_size starti, endi, j;
					g_scale_value(sample_count, i, rc_client->right, starti, endi, m_scale == scale_logarithmic);
					for (j=starti;j<=endi; j++)
					{
						if (j < sample_count)
						{
							double sample_val = 0;
							t_size k;
							for (k = 0; k<channel_count; k++)
								sample_val += p_data[j*channel_count + k];
							sample_val *= 1.0 / channel_count;
							val = max(val, sample_val);
						}

					}
					RECT r;
					r.left = i;
					r.right = i+1;
					r.bottom = rc_client->bottom;
					r.top = rc_client->bottom - g_scale_value_single (val, rc_client->bottom, m_vertical_scale == scale_logarithmic);
					if (r.bottom>r.top)
						FillRect(dc,&r,br_foreground);
				}
			}
		}
	}
	ps.release();
}

void spectrum_extension::g_refresh_all()
{
	double p_time = NULL;
	g_stream->get_absolute_time(p_time);
	audio_chunk_impl p_chunk;

	unsigned fft_size = 4096;
	bool ret = g_stream->get_spectrum_absolute(p_chunk, p_time, fft_size);

	unsigned n, count = spectrum_extension::g_visualisations.get_count();
	for (n=0; n<count; n++)
	{
		spectrum_extension * vis_ext = spectrum_extension::g_visualisations[n];
		if (ret)
			vis_ext->refresh(&p_chunk);
	}
}

void spectrum_extension::get_name(pfc::string_base & out)const
{
	out.set_string("Spectrum analyser");
}

void spectrum_extension::set_config(stream_reader * r, t_size p_size, abort_callback & p_abort)
{
	if (p_size)
	{
		r->read_lendian_t(cr_fore, p_abort);
		r->read_lendian_t(cr_back, p_abort);
		r->read_lendian_t(mode, p_abort);
		try {
		r->read_lendian_t(m_scale, p_abort);
		r->read_lendian_t(m_vertical_scale, p_abort);
		} catch (const exception_io_data_truncation &)
		{};
	}
}


void spectrum_extension::get_config(stream_writer * data, abort_callback & p_abort) const
{
	data->write_lendian_t(cr_fore, p_abort);
	data->write_lendian_t(cr_back, p_abort);
	data->write_lendian_t(mode, p_abort);
	data->write_lendian_t(m_scale, p_abort);
	data->write_lendian_t(m_vertical_scale, p_abort);
}

// {D947777C-94C7-409a-B02C-9B0EB9E374FA}
const GUID spectrum_extension::extension_guid = 
{ 0xd947777c, 0x94c7, 0x409a, { 0xb0, 0x2c, 0x9b, 0xe, 0xb9, 0xe3, 0x74, 0xfa } };

ui_extension::visualisation_factory<spectrum_extension> blah;

class window_visualisation_spectrum : public window_visualisation
{
	const GUID & get_visualisation_guid() const override
	{
		return spectrum_extension::extension_guid;
	}
	const GUID & get_extension_guid() const override
	{
		return spectrum_extension::extension_guid;
	}
	void get_menu_items (ui_extension::menu_hook_t & p_hook) override
	{
		p_hook.add_node(uie::menu_node_ptr(new(std::nothrow) uie::menu_node_configure(this)));
	};
	void set_config(stream_reader * r, t_size p_size, abort_callback & p_abort) override
	{
		if (p_size)
		{
			t_uint32 m_frame;
			r->read_lendian_t(m_frame, p_abort);
			{
				set_frame_style(m_frame);
				unsigned size = 0;
				r->read_lendian_t(size, p_abort);
				pfc::array_t<t_uint8> m_data;
				m_data.set_size(size);
				r->read(m_data.get_ptr(), size, p_abort);
				set_vis_data(m_data.get_ptr(), m_data.get_size());
			}
		}
	}
	void get_config(stream_writer * data, abort_callback & p_abort) const override
	{
		pfc::array_t<t_uint8> m_data;
		data->write_lendian_t(get_frame_style(), p_abort);
		get_vis_data(m_data);
		data->write_lendian_t(m_data.get_size(), p_abort);
		data->write(m_data.get_ptr(), m_data.get_size(), p_abort);
	}
	bool have_config_popup()const override{return true;}
	bool show_config_popup(HWND wnd_parent) override
	{
		uie::visualisation_ptr p_vis;
		service_ptr_t<spectrum_extension> p_this;
		get_vis_ptr(p_vis);
		if (p_vis.is_valid())
			p_this = static_cast<spectrum_extension*>(p_vis.get_ptr());

		service_ptr_t<spectrum_extension> p_temp = p_this;
		if (!p_temp.is_valid())
			uie::visualization::create_by_guid(get_visualisation_guid(), reinterpret_cast<uie::visualisation_ptr &>(p_temp));

		pfc::array_t<t_uint8> m_data;
		if (!p_temp->b_active)
		{
			try {
				abort_callback_dummy p_abort;
				get_vis_data(m_data);
				p_temp->set_config_from_ptr(m_data.get_ptr(), m_data.get_size(), p_abort);
			} catch (const exception_io &) {};
		}

		spec_param param(p_temp->cr_fore, p_temp->cr_back, p_temp->mode, p_temp->m_scale, p_temp->m_vertical_scale, p_temp.get_ptr(), true, get_frame_style());

		bool rv = !!uDialogBox(IDD_POPUP_SPECTRUM_NEW, wnd_parent, SpectrumPopupProc, (LPARAM)(&param));
		if (rv)
		{
			p_temp->cr_fore = param.cr_fore;
			cfg_vis2 = param.cr_fore;
			p_temp->cr_back = param.cr_back;
			cfg_vis = param.cr_back;
			p_temp->mode = param.mode;
			cfg_vis_mode = param.mode;
			p_temp->m_scale = param.m_scale;
			cfg_scale = param.m_scale;
			p_temp->m_vertical_scale = param.m_vertical_scale;
			cfg_vertical_scale = param.m_vertical_scale;
			set_frame_style(param.frame);
			if (p_temp->b_active)
			{
				p_temp->flush_brushes();
				p_temp->clear();
			}
			else
			{
				try {
					abort_callback_dummy p_abort;
					p_temp->get_config_to_array(m_data, p_abort, true);
					set_vis_data(m_data.get_ptr(), m_data.get_size());
				}
				catch (pfc::exception &)
				{};
			}

		}
		return rv;
	}
};

ui_extension::window_factory<window_visualisation_spectrum> blahg;
#endif