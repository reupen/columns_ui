#include "stdafx.h"



bool titleformat_hook_playlist_name::process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag)
{
	if (p_name_length && *p_name == '_')
	{
		p_name++;
		p_name_length--;
	}
	if (!stricmp_utf8_ex(p_name, p_name_length, "playlist_name", pfc_infinite))
	{
		initialise();
		p_out->write(titleformat_inputtypes::unknown, m_name);
		p_found_flag = true;
		return true;
	}
	return false;
}



void titleformat_hook_playlist_name::initialise()
{
	if (!m_initialised)
	{
		static_api_ptr_t<playlist_manager_v3>()->activeplaylist_get_name(m_name);
		m_initialised = true;
	}
}

bool titleformat_hook_style::process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag)
{
	p_found_flag = false;
	{
		if (p_name_length > 1 && !stricmp_utf8_ex(p_name, 1, "_", pfc_infinite))
		{
			if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "text", pfc_infinite))
			{
				if (!text.get_ptr())
				{
					text.set_size(33);
					text.fill(0);
					_ultoa_s(p_default_colours.text_colour, text.get_ptr(), text.get_size(), 0x10);
				}
				p_out->write(titleformat_inputtypes::unknown, text.get_ptr(), text.get_size());
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_text", pfc_infinite))
			{
				if (!selected_text.get_ptr())
				{
					selected_text.set_size(33);
					selected_text.fill(0);
					_ultoa_s(p_default_colours.selected_text_colour, selected_text.get_ptr(), selected_text.get_size(), 0x10);
				}
				p_out->write(titleformat_inputtypes::unknown, selected_text.get_ptr(), selected_text.get_size());
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "back", pfc_infinite))
			{
				if (!back.get_ptr())
				{
					back.set_size(33);
					back.fill(0);
					_ultoa_s(p_default_colours.background_colour, back.get_ptr(), back.get_size(), 0x10);
				}
				p_out->write(titleformat_inputtypes::unknown, back.get_ptr(), back.get_size());
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_back", pfc_infinite))
			{
				if (!selected_back.get_ptr())
				{
					selected_back.set_size(33);
					selected_back.fill(0);
					_ultoa_s(p_default_colours.selected_background_colour, selected_back.get_ptr(), selected_back.get_size(), 0x10);
				}
				p_out->write(titleformat_inputtypes::unknown, selected_back.get_ptr(), selected_back.get_size());
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_back_no_focus", pfc_infinite))
			{
				if (!selected_back_no_focus.get_ptr())
				{
					selected_back_no_focus.set_size(33);
					selected_back_no_focus.fill(0);
					_ultoa_s(p_default_colours.selected_background_colour_non_focus, selected_back_no_focus.get_ptr(), selected_back_no_focus.get_size(), 0x10);
				}
				p_out->write(titleformat_inputtypes::unknown, selected_back_no_focus.get_ptr(), selected_back_no_focus.get_size());
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "selected_text_no_focus", pfc_infinite))
			{
				if (!selected_text_no_focus.get_ptr())
				{
					selected_text_no_focus.set_size(33);
					selected_text_no_focus.fill(0);
					_ultoa_s(p_default_colours.selected_text_colour_non_focus, selected_text_no_focus.get_ptr(), selected_text_no_focus.get_size(), 0x10);
				}
				p_out->write(titleformat_inputtypes::unknown, selected_text_no_focus.get_ptr(), selected_text_no_focus.get_size());
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 1, p_name_length - 1, "themed", pfc_infinite))
			{
				cui::colours::helper p_helper(appearance_client_pv_impl::g_guid);
				if (p_helper.get_themed())
				{
					p_out->write(titleformat_inputtypes::unknown, "1", 1);
					p_found_flag = true;
				}
				return true;
			}
		}
	}

	return false;
}

bool titleformat_hook_style::process_function(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag)
{
	p_found_flag = false;
	if (!stricmp_utf8_ex(p_name, p_name_length, "set_style", pfc_infinite))
	{
		if (p_params->get_param_count() >= 2)
		{
			const char * name;
			unsigned name_length;
			p_params->get_param(0, name, name_length);
			if (!stricmp_utf8_ex(name, name_length, "text", pfc_infinite))
			{
				{
					const char * value;
					unsigned value_length;
					p_params->get_param(1, value, value_length);
					if (value_length && *value == 3)
					{
						value++;
						value_length--;
					}
					p_colours.text_colour.set(strtoul_n(value, value_length, 0x10));
					if (value_length == 6 * 2 + 2 && value[6] == '|')
					{
						value += 7;
						value_length -= 7;
						p_colours.selected_text_colour.set(strtoul_n(value, value_length, 0x10));
					}
					else
						p_colours.selected_text_colour.set(0xffffff - p_colours.text_colour);
				}
				if (p_params->get_param_count() >= 3)
				{
					{
						const char * value;
						unsigned value_length;
						p_params->get_param(2, value, value_length);
						if (value_length && *value == 3)
						{
							value++;
							value_length--;
						}
						p_colours.selected_text_colour.set(strtoul_n(value, value_length, 0x10));
					}
				}
				if (p_params->get_param_count() >= 4)
				{
					const char * value;
					unsigned value_length;
					p_params->get_param(3, value, value_length);
					if (value_length && *value == 3)
					{
						value++;
						value_length--;
					}
					p_colours.selected_text_colour_non_focus.set(strtoul_n(value, value_length, 0x10));
				}
				else p_colours.selected_text_colour_non_focus.set(p_colours.selected_text_colour);
			}
			else if (!stricmp_utf8_ex(name, name_length, "back", pfc_infinite))
			{
				{
					const char * value;
					unsigned value_length;
					p_params->get_param(1, value, value_length);
					if (value_length && *value == 3)
					{
						value++;
						value_length--;
					}
					p_colours.background_colour.set(strtoul_n(value, value_length, 0x10));

					if (value_length == 6 * 2 + 2 && value[6] == '|')
					{
						value += 7;
						value_length -= 7;
						p_colours.selected_background_colour.set(strtoul_n(value, value_length, 0x10));
					}
					else
						p_colours.selected_background_colour.set(0xffffff - p_colours.background_colour);
				}
				if (p_params->get_param_count() >= 3)
				{
					{
						const char * value;
						unsigned value_length;
						p_params->get_param(2, value, value_length);
						if (value_length && *value == 3)
						{
							value++;
							value_length--;
						}
						p_colours.selected_background_colour.set(strtoul_n(value, value_length, 0x10));
					}
					if (p_params->get_param_count() >= 4)
					{
						const char * value;
						unsigned value_length;
						p_params->get_param(3, value, value_length);
						if (value_length && *value == 3)
						{
							value++;
							value_length--;
						}
						p_colours.selected_background_colour_non_focus.set(strtoul_n(value, value_length, 0x10));
					}
					else p_colours.selected_background_colour_non_focus.set(p_colours.selected_background_colour);
				}
				else p_colours.selected_background_colour_non_focus.set(p_colours.selected_background_colour);
			}
			else if (name_length >= 6 && !stricmp_utf8_ex(name, 6, "frame-", pfc_infinite))
			{
				const char * p_side = name;
				p_side += 6;
				if (!stricmp_utf8_ex(p_side, name_length - 6, "left", pfc_infinite))
				{
					const char * value;
					unsigned value_length;
					p_params->get_param(1, value, value_length);
					p_colours.use_frame_left = (value_length == 1 && *value == '1');
					if (p_params->get_param_count() >= 3)
					{
						{
							const char * value;
							unsigned value_length;
							p_params->get_param(2, value, value_length);
							if (value_length && *value == 3)
							{
								value++;
								value_length--;
							}
							p_colours.frame_left.set(strtoul_n(value, value_length, 0x10));
						}
					}
				}
				else if (!stricmp_utf8_ex(p_side, name_length - 6, "top", pfc_infinite))
				{
					const char * value;
					unsigned value_length;
					p_params->get_param(1, value, value_length);
					p_colours.use_frame_top = (value_length == 1 && *value == '1');
					if (p_params->get_param_count() >= 3)
					{
						{
							const char * value;
							unsigned value_length;
							p_params->get_param(2, value, value_length);
							if (value_length && *value == 3)
							{
								value++;
								value_length--;
							}
							p_colours.frame_top.set(strtoul_n(value, value_length, 0x10));
						}
					}
				}
				else if (!stricmp_utf8_ex(p_side, name_length - 6, "right", pfc_infinite))
				{
					const char * value;
					unsigned value_length;
					p_params->get_param(1, value, value_length);
					p_colours.use_frame_right = (value_length == 1 && *value == '1');
					if (p_params->get_param_count() >= 3)
					{
						{
							const char * value;
							unsigned value_length;
							p_params->get_param(2, value, value_length);
							if (value_length && *value == 3)
							{
								value++;
								value_length--;
							}
							p_colours.frame_right.set(strtoul_n(value, value_length, 0x10));
						}
					}
				}
				else if (!stricmp_utf8_ex(p_side, name_length - 6, "bottom", pfc_infinite))
				{
					const char * value;
					unsigned value_length;
					p_params->get_param(1, value, value_length);
					p_colours.use_frame_bottom = (value_length == 1 && *value == '1');
					if (p_params->get_param_count() >= 3)
					{
						{
							const char * value;
							unsigned value_length;
							p_params->get_param(2, value, value_length);
							if (value_length && *value == 3)
							{
								value++;
								value_length--;
							}
							p_colours.frame_bottom.set(strtoul_n(value, value_length, 0x10));
						}
					}
				}
			}
			p_found_flag = true;
			return true;
		}
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "calculate_blend_target", pfc_infinite))
	{
		if (p_params->get_param_count() == 1)
		{
			const char * p_val;
			unsigned p_val_length;
			p_params->get_param(0, p_val, p_val_length);

			int colour = strtoul_n(p_val, p_val_length, 0x10);
			int total = (colour & 0xff)
				+ ((colour & 0xff00) >> 8)
				+ ((colour & 0xff0000) >> 16);

			COLORREF blend_target = total >= 128 * 3 ? 0 : 0xffffff;

			char temp[33];
			memset(temp, 0, 33);
			_ultoa_s(blend_target, temp, 16);
			p_out->write(titleformat_inputtypes::unknown, temp, 33);
			p_found_flag = true;
			return true;
		}
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "offset_colour", pfc_infinite))
	{
		if (p_params->get_param_count() == 3)
		{
			const char * p_val, *p_val2;
			unsigned p_val_length, p_val2_length;
			p_params->get_param(0, p_val, p_val_length);
			int colour = strtoul_n(p_val, p_val_length, 0x10);
			p_params->get_param(1, p_val2, p_val2_length);
			int target = strtoul_n(p_val2, p_val2_length, 0x10);
			int amount = p_params->get_param_uint(2);

			int rdiff = (target & 0xff) - (colour & 0xff);
			int gdiff = ((target & 0xff00) >> 8) - ((colour & 0xff00) >> 8);
			int bdiff = ((target & 0xff0000) >> 16) - ((colour & 0xff0000) >> 16);

			int totaldiff = abs(rdiff) + abs(gdiff) + abs(bdiff);

			int newr = (colour & 0xff) + (totaldiff ? (rdiff * amount * 3 / totaldiff) : 0);
			if (newr < 0) newr = 0;
			if (newr > 255) newr = 255;

			int newg = ((colour & 0xff00) >> 8) + (totaldiff ? (gdiff * amount * 3 / totaldiff) : 0);
			if (newg < 0) newg = 0;
			if (newg > 255) newg = 255;

			int newb = ((colour & 0xff0000) >> 16) + (totaldiff ? (bdiff * amount * 3 / totaldiff) : 0);
			if (newb < 0) newb = 0;
			if (newb > 255) newb = 255;

			int newrgb = RGB(newr, newg, newb);

			char temp[33];
			memset(temp, 0, 33);

			_ultoa_s(newrgb, temp, 16);
			p_out->write(titleformat_inputtypes::unknown, temp, 33);
			p_found_flag = true;
			return true;
		}
	}
	return false;
}



int date_to_julian(const SYSTEMTIME * st)
{
	int year = st->wYear - 1;
	int hour = st->wHour;
	int day = st->wDay - (st->wHour > 11 ? 0 : 1);
	int monthdays = 0;
	switch (st->wMonth)
	{
	case 2:
		monthdays = 31;
		break;
	case 3:
		monthdays = 59;
		break;
	case 4:
		monthdays = 90;
		break;
	case 5:
		monthdays = 120;
		break;
	case 6:
		monthdays = 151;
		break;
	case 7:
		monthdays = 181;
		break;
	case 8:
		monthdays = 212;
		break;
	case 9:
		monthdays = 243;
		break;
	case 10:
		monthdays = 273;
		break;
	case 11:
		monthdays = 304;
		break;
	case 12:
		monthdays = 334;
		break;
	}

	int yeardays = monthdays;
	if ((st->wYear % 4) && (!(st->wYear % 100) || (st->wYear % 400))) yeardays++;

	return (year / 4 - year / 100 + year / 400) + (year * 365) + yeardays + day + 1721395 + 29;

}

#if 0
double dateToJulian(SYSTEMTIME *st)
{
	double jy, ja, jm;

	if (st->wYear == 0)
	{
		//alert("There is no year 0 in the Julian system!");
		return -1;
	}

	// check for invalid date within date change - disabled for now...
	// not likely to happen in foobar.
	//if( st->wYear == 1582 && st->wMonth == 10 && st->wDay > 4 && st->wDay < 15)
	//	return -1;

	int year = st->wYear;

	// also disabling CE vs. BCE handling... again, not likely to happen
	//if( !date.ce ) 
	//	year = -year + 1;

	if (st->wMonth > 2)
	{
		jy = year;
		jm = st->wMonth + 1;
	}
	else
	{
		jy = year - 1;
		jm = st->wMonth + 13;
	}

	double intgr = floor(floor(365.25*jy) + floor(30.6001*jm) + st->wDay + 1720995);

	//check for switch to Gregorian calendar
	double gregcal = 15 + 31 * (10 + 12 * 1582);
	if (st->wDay + 31 * (st->wMonth + 12 * year) >= gregcal)
	{
		ja = floor(0.01*jy);
		intgr += 2 - ja + floor(0.25*ja);
	}

	//correct for half-day offset
	double dayfrac = st->wHour / 24.0 - 0.5;
	if (dayfrac < 0.0)
	{
		dayfrac += 1.0;
		--intgr;
	}

	//now set the fraction of a day
	dayfrac = dayfrac + (st->wMinute + st->wSecond / 60.0) / 60.0 / 24.0;

	return intgr + dayfrac;
}
#endif

bool titleformat_hook_date::process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag)
{
	p_found_flag = false;

	if (p_st)
	{
		if (p_name_length > 8 && !stricmp_utf8_ex(p_name, 8, "_system_", pfc_infinite))
		{
			if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "year", pfc_infinite))
			{
				if (!year.get_ptr())
				{
					year.set_size(33);
					year.fill(0);
					_ultoa_s(p_st->wYear, year.get_ptr(), year.get_size(), 10);
				}
				p_out->write(titleformat_inputtypes::unknown, year.get_ptr(), pfc_infinite);
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "month", pfc_infinite))
			{
				if (!month.get_ptr())
				{
					month.set_size(33);
					month.fill(0);
					_ultoa_s(p_st->wMonth, month.get_ptr(), month.get_size(), 10);
				}
				p_out->write(titleformat_inputtypes::unknown, month.get_ptr(), pfc_infinite);
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "day", pfc_infinite))
			{
				if (!day.get_ptr())
				{
					day.set_size(33);
					day.fill(0);
					_ultoa_s(p_st->wDay, day.get_ptr(), day.get_size(), 10);
				}
				p_out->write(titleformat_inputtypes::unknown, day.get_ptr(), pfc_infinite);
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "day_of_week", pfc_infinite))
			{
				if (!dayofweek.get_ptr())
				{
					dayofweek.set_size(33);
					dayofweek.fill(0);
					_ultoa_s(p_st->wDayOfWeek + 1, dayofweek.get_ptr(), dayofweek.get_size(), 10);
				}
				p_out->write(titleformat_inputtypes::unknown, dayofweek.get_ptr(), pfc_infinite);
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "julian_day", pfc_infinite))
			{
				if (!julian.get_ptr())
				{
					julian.set_size(33);
					julian.fill(0);
					_ultoa_s(date_to_julian(p_st), julian.get_ptr(), julian.get_size(), 10);
				}
				p_out->write(titleformat_inputtypes::unknown, julian.get_ptr(), pfc_infinite);
				p_found_flag = true;
				return true;
			}
			else if (!stricmp_utf8_ex(p_name + 8, p_name_length - 8, "hour", pfc_infinite))
			{
				if (!hour.get_ptr())
				{
					hour.set_size(33);
					hour.fill(0);
					_ultoa_s(p_st->wHour, hour.get_ptr(), hour.get_size(), 10);
				}
				p_out->write(titleformat_inputtypes::unknown, hour.get_ptr(), pfc_infinite);
				p_found_flag = true;
				return true;
			}
		}
	}

	return false;
}


bool titleformat_hook_splitter_pt3::process_field(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, bool & p_found_flag)
{
	p_found_flag = false;
	if (m_hook1 && m_hook1->process_field(p_out, p_name, p_name_length, p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook2 && m_hook2->process_field(p_out, p_name, p_name_length, p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook3 && m_hook3->process_field(p_out, p_name, p_name_length, p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook4 && m_hook4->process_field(p_out, p_name, p_name_length, p_found_flag)) return true;
	p_found_flag = false;
	return false;
}

bool titleformat_hook_splitter_pt3::process_function(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag)
{
	p_found_flag = false;
	if (m_hook1 && m_hook1->process_function(p_out, p_name, p_name_length, p_params, p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook2 && m_hook2->process_function(p_out, p_name, p_name_length, p_params, p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook3 && m_hook3->process_function(p_out, p_name, p_name_length, p_params, p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook4 && m_hook4->process_function(p_out, p_name, p_name_length, p_params, p_found_flag)) return true;
	p_found_flag = false;
	return false;
}


bool titleformat_hook_date::process_function(titleformat_text_out * p_out, const char * p_name, unsigned p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag)
{
	p_found_flag = false;
	return false;
}
