#pragma once

#include "dark_mode.h"
#include "dark_mode_trackbar.h"
#include "system_appearance_manager.h"

template <bool b_vertical, bool b_popup, typename t_attributes, class t_base = ui_helpers::container_window>
class VolumeBar
    : public t_base
    , private play_callback {
    class VolumeTrackBar : public uih::Trackbar {
        void get_channel_rect(RECT* rc) const override
        {
            if (b_popup)
                Trackbar::get_channel_rect(rc);
            else {
                RECT rc_client;
                GetClientRect(get_wnd(), &rc_client);
                const auto cx = calculate_thumb_size();

                rc->left = get_orientation() ? rc_client.left + 5 : rc_client.left + cx / 2;
                rc->right = get_orientation() ? rc_client.right - 5 : rc_client.right - cx + cx / 2;
                rc->top = get_orientation() ? rc_client.top + cx / 2 : rc_client.top + 5;
                rc->bottom = get_orientation() ? rc_client.bottom - cx + cx / 2 : rc_client.bottom - 5;
            }
        }

        void draw_channel(HDC dc, const RECT* rc) const override
        {
            if (b_popup)
                Trackbar::draw_channel(dc, rc);
            else {
                const auto is_dark = cui::colours::is_dark_mode_active();
                COLORREF cr_shadow = cui::dark::get_system_colour(COLOR_3DSHADOW, is_dark);
                COLORREF cr_hilight = cui::dark::get_system_colour(COLOR_3DHILIGHT, is_dark);

                if (m_this->get_using_gdiplus()) {
                    Gdiplus::Graphics graphics(dc);
                    if (Gdiplus::Ok == graphics.GetLastStatus()) {
                        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
                        RECT rcdraw(*rc);

                        Gdiplus::Color colour_hilight, colour_shadow;
                        colour_hilight.SetFromCOLORREF(cr_hilight);
                        colour_shadow.SetFromCOLORREF(cr_shadow);

                        Gdiplus::Pen pen_hilight(colour_hilight);
                        Gdiplus::Pen pen_shadow(colour_shadow);

                        graphics.DrawLine(&pen_shadow, Gdiplus::Point{rcdraw.left, rcdraw.bottom - 1},
                            Gdiplus::Point{rcdraw.right, rcdraw.top});
                        graphics.DrawLine(&pen_hilight, Gdiplus::Point{rcdraw.right, rcdraw.top},
                            Gdiplus::Point{rcdraw.right, rcdraw.bottom});
                        graphics.DrawLine(&pen_hilight, Gdiplus::Point{rcdraw.right, rcdraw.bottom},
                            Gdiplus::Point{rcdraw.left - 1, rcdraw.bottom});
                    }
                } else {
                    HPEN pn_light = CreatePen(PS_SOLID, 1, cr_hilight);
                    HPEN pn_shadow = CreatePen(PS_SOLID, 1, cr_shadow);

                    HPEN pn_old = SelectPen(dc, pn_shadow);
                    MoveToEx(dc, rc->left, rc->bottom - 1, nullptr);
                    LineTo(dc, rc->right, rc->top);

                    SelectPen(dc, pn_light);
                    LineTo(dc, rc->right, rc->bottom);
                    LineTo(dc, rc->left - 1, rc->bottom);

                    SelectPen(dc, pn_old);

                    DeleteObject(pn_light);
                    DeleteObject(pn_shadow);
                }
            }
        }
        VolumeBar<b_vertical, b_popup, t_attributes, t_base>* m_this;

    public:
        explicit VolumeTrackBar(VolumeBar<b_vertical, b_popup, t_attributes, t_base>* p_this) : m_this(p_this) {}
    } m_child;

    class VolumeTrackBarCallback : public uih::TrackbarCallback {
        void on_position_change(unsigned pos, bool b_tracking) override
        {
            const auto volume = position_to_volume(pos);
            playback_control::get()->set_volume(static_cast<float>(volume));
        }

        void get_tooltip_text(unsigned pos, uih::TrackbarString& out) override
        {
            const auto volume = position_to_volume(pos);
            out.append(pfc::stringcvt::string_os_from_utf8(pfc::format_float(volume, 0, 2)));
            out.append(_T(" dB"));
        }

        bool on_key(WPARAM wp, LPARAM lp) override
        {
            if (b_popup && wp == VK_ESCAPE && !(lp & (1 << 31))) {
                PostMessage(m_this->get_wnd(), WM_CLOSE, 0, 0);
                return true;
            }
            return false;
        }
        VolumeBar<b_vertical, b_popup, t_attributes, t_base>* m_this;

    public:
        explicit VolumeTrackBarCallback(VolumeBar<b_vertical, b_popup, t_attributes, t_base>* p_this) : m_this(p_this)
        {
        }
    } m_track_bar_host;

public:
    static double position_to_volume(const unsigned position)
    {
        const auto normalised = position / 1000.0;
        auto volume = 10.0 * std::log2(normalised);
        if (volume < -100.0)
            volume = -100;

        return volume;
    }

    static unsigned volume_to_position(const double volume)
    {
        auto position = std::lround(std::pow(2.0, volume / 10.0) * 1000.0);
        if (position < 0)
            position = 0;
        if (position > 1000)
            position = 1000;

        return gsl::narrow<unsigned>(position);
    }

    bool get_using_gdiplus() { return m_using_gdiplus; }

    void set_custom_colours()
    {
        if (cui::colours::is_dark_mode_active())
            m_child.set_custom_colours(cui::dark::get_dark_trackbar_colours());
        else
            m_child.set_custom_colours({});
    }

    LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) override
    {
        switch (msg) {
        case WM_CREATE: {
            if (!b_popup) {
                Gdiplus::GdiplusStartupInput gdiplusStartupInput;
                if (Gdiplus::Ok == GdiplusStartup(&m_Gdiplus_token, &gdiplusStartupInput, nullptr))
                    m_using_gdiplus = true;
            }
            if (t_attributes::get_show_caption())
                m_font_caption.reset(uCreateMenuFont(b_vertical));

            m_child.set_callback(&m_track_bar_host);
            m_child.set_show_tooltips(true);

            set_custom_colours();

            m_dark_mode_notifier = std::make_unique<cui::colours::dark_mode_notifier>([this] { set_custom_colours(); });

            m_modern_colours_changed_token = cui::system_appearance_manager::add_modern_colours_change_handler([this] {
                if (cui::colours::is_dark_mode_active())
                    set_custom_colours();
            });

            wnd_trackbar = m_child.create(wnd);

            if (wnd_trackbar) {
                m_child.set_orientation(b_vertical);
                m_child.set_direction(b_vertical);
                m_child.set_range(1000);
                m_child.set_auto_focus(b_popup);
                m_child.set_scroll_step(20);
                m_child.set_mouse_wheel_direction(!b_vertical);
                update_position();
                if (b_popup)
                    SetFocus(wnd_trackbar);
            }
            ShowWindow(wnd_trackbar, SW_SHOWNORMAL);

            play_callback_manager::get()->register_callback(this, flag_on_volume_change, false);

            break;
        }
        case WM_SIZE: {
            if (t_attributes::get_show_caption() && b_popup && b_vertical) {
                const auto size_caption = get_caption_size();
                const auto left = size_caption + 2 * 1_spx;
                const auto top = 3_spx + 1_spx;
                const auto right = LOWORD(lp) - 2 * 1_spx;
                const auto bottom = HIWORD(lp) - 1_spx - 3_spx;
                const auto width = right - left;
                const auto height = bottom - top;
                SetWindowPos(wnd_trackbar, nullptr, left, top, width, height, SWP_NOZORDER);
                RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE);
            } else
                SetWindowPos(wnd_trackbar, nullptr, 0, 0, LOWORD(lp), HIWORD(lp), SWP_NOZORDER);
        } break;
        case WM_GETMINMAXINFO: {
            auto mmi = LPMINMAXINFO(lp);

            if (!b_popup) {
                if (!b_vertical)
                    mmi->ptMinTrackSize.y = uih::scale_dpi_value(21);
                mmi->ptMinTrackSize.x = uih::scale_dpi_value(50);
                // mmi->ptMaxTrackSize.y = 20;
            }
        }
            return 0;
        case WM_PRINTCLIENT: {
            if (lp & PRF_ERASEBKGND) {
                if (!b_popup) {
                    auto dc = (HDC)wp;
                    HWND wnd_parent = GetAncestor(wnd, GA_PARENT);
                    POINT pt = {0, 0}, pt_old = {0, 0};
                    MapWindowPoints(wnd, wnd_parent, &pt, 1);
                    OffsetWindowOrgEx(dc, pt.x, pt.y, &pt_old);
                    SendMessage(wnd_parent, msg, wp, lp);
                    SetWindowOrgEx(dc, pt_old.x, pt_old.y, nullptr);
                }
            }
        } break;
        case WM_ERASEBKGND: {
            // Note: In non-pop-up mode, this is handled by uie::container_window
            RECT rect{};
            GetClientRect(wnd, &rect);
            const auto brush = cui::dark::get_colour_brush(
                cui::dark::ColourID::VolumePopupBackground, cui::colours::is_dark_mode_active());
            FillRect(reinterpret_cast<HDC>(wp), &rect, brush.get());
            return TRUE;
        }
        case WM_PAINT: {
            const auto is_dark = cui::colours::is_dark_mode_active();

            PAINTSTRUCT ps{};
            const auto _ = wil::BeginPaint(wnd, &ps);

            if (b_popup) {
                RECT rect{};
                GetClientRect(wnd, &rect);

                const auto border_colour = cui::dark::get_colour(cui::dark::ColourID::VolumePopupBorder, is_dark);
                const auto pen = wil::unique_hpen(CreatePen(PS_INSIDEFRAME, 1_spx, border_colour));
                const auto _select_pen = wil::SelectObject(ps.hdc, pen.get());
                const auto _select_brush = wil::SelectObject(ps.hdc, GetStockObject(NULL_BRUSH));
                Rectangle(ps.hdc, rect.left, rect.top, rect.right, rect.bottom);
            }

            if (t_attributes::get_show_caption()) {
                RECT rc_client, rc_dummy;
                GetClientRect(wnd, &rc_client);
                SIZE sz{};
                get_caption_extent(sz);
                const auto size_caption = get_caption_size();
                const auto left = 1_spx + 1_spx;
                const RECT rc_caption = {left, 1_spx, left + size_caption, rc_client.bottom - 9_spx - 1_spx};

                if (IntersectRect(&rc_dummy, &rc_caption, &ps.rcPaint)) {
                    SetBkMode(ps.hdc, TRANSPARENT);
                    const auto text_colour = get_colour(cui::dark::ColourID::VolumePopupText, is_dark);
                    SetTextColor(ps.hdc, text_colour);

                    const auto _ = wil::SelectObject(ps.hdc, m_font_caption.get());
                    uExtTextOut(ps.hdc, rc_caption.left, rc_caption.bottom, ETO_CLIPPED, &rc_caption, label_text.data(),
                        label_text.length(), nullptr);
                }
            }
            return 0;
        }
        case WM_KEYDOWN: {
            auto lpkeyb = uih::GetKeyboardLParam(lp);
            if (b_popup && wp == VK_ESCAPE && !lpkeyb.transition_code && !lpkeyb.previous_key_state) {
                PostMessage(wnd, WM_CLOSE, 0, 0);
            }
        } break;
        case WM_CLOSE:
            DestroyWindow(wnd);
            return 0;
        case WM_ACTIVATE: {
            if (b_popup) {
                if (LOWORD(wp) == WA_INACTIVE)
                    PostMessage(wnd, WM_CLOSE, 0, 0);
            }
        } break;
        case WM_DESTROY: {
            m_modern_colours_changed_token.reset();
            m_dark_mode_notifier.reset();
            play_callback_manager::get()->unregister_callback(this);
            m_child.destroy();
            if (t_attributes::get_show_caption())
                m_font_caption.reset();

            if (!b_popup && m_using_gdiplus) {
                Gdiplus::GdiplusShutdown(m_Gdiplus_token);
                m_using_gdiplus = false;
                m_Gdiplus_token = NULL;
            }
        } break;
        }
        return DefWindowProc(wnd, msg, wp, lp);
    }

    VolumeBar() : m_child(this), m_track_bar_host(this) {}
    VolumeBar(const VolumeBar&) = delete;
    VolumeBar& operator=(const VolumeBar&) = delete;
    VolumeBar(VolumeBar&&) = delete;
    VolumeBar& operator=(VolumeBar&&) = delete;
    ~VolumeBar() = default;

    ui_helpers::container_window::class_data& get_class_data() const override
    {
        __implement_get_class_data_ex(t_attributes::get_class_name(), _T(""), !b_popup, 0,
            b_popup ? WS_POPUP | WS_CLIPCHILDREN : WS_CHILD | WS_CLIPCHILDREN,
            b_popup ? WS_EX_TOOLWINDOW | WS_EX_TOPMOST : WS_EX_CONTROLPARENT, 0);
    }

    void update_position()
    {
        float vol = playback_control::get()->get_volume();
        update_position(vol);
    }
    void update_position(float p_new_volume) { m_child.set_position(volume_to_position(p_new_volume)); }
    static int g_get_caption_size(HFONT fnt)
    {
        if (!t_attributes::get_show_caption())
            return 0;

        return gsl::narrow<int>(uGetFontHeight(fnt)) + 1_spx;
    }
    static int g_get_caption_size()
    {
        if (!t_attributes::get_show_caption())
            return 0;
        wil::unique_hfont fnt(uCreateMenuFont());
        unsigned rv = g_get_caption_size(fnt.get());
        return rv;
    }

private:
    int get_caption_size() const { return g_get_caption_size(m_font_caption.get()); }
    bool get_caption_extent(SIZE& p_out) const
    {
        const auto dc = wil::GetDC(this->get_wnd());
        const auto _ = wil::SelectObject(dc.get(), m_font_caption.get());
        return uGetTextExtentPoint32(dc.get(), label_text.data(), label_text.size(), &p_out) != 0;
    }

    void FB2KAPI on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {}
    void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track) override {}
    void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason) override {}
    void FB2KAPI on_playback_seek(double p_time) override {}
    void FB2KAPI on_playback_pause(bool p_state) override {}
    void FB2KAPI on_playback_edited(metadb_handle_ptr p_track) override {}
    void FB2KAPI on_playback_dynamic_info(const file_info& p_info) override {}
    void FB2KAPI on_playback_dynamic_info_track(const file_info& p_info) override {}
    void FB2KAPI on_playback_time(double p_time) override {}
    void FB2KAPI on_volume_change(float p_new_val) override { update_position(p_new_val); }

    constexpr static auto label_text = "Volume"sv;

    HWND wnd_trackbar{nullptr};
    wil::unique_hfont m_font_caption;
    ULONG_PTR m_Gdiplus_token{NULL};
    bool m_using_gdiplus{false};
    std::unique_ptr<cui::colours::dark_mode_notifier> m_dark_mode_notifier;
    std::unique_ptr<cui::system_appearance_manager::EventToken> m_modern_colours_changed_token;
};

class PopupVolumeBarAttributes {
public:
    static const TCHAR* get_class_name() { return _T("volume_popup"); }
    static bool get_show_caption() { return true; }
};

using volume_popup_t = VolumeBar<true, true, PopupVolumeBarAttributes>;
