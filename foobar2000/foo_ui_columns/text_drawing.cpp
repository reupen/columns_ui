#include "foo_ui_columns.h"

//#define _WIN32_IE 0x0501

#if 0

#define ELLIPSIS "\xe2\x80\xa6"//"\x85"
#define ELLIPSIS_LEN 3

bool is_win2k_or_newer()
{
	OSVERSIONINFO ov;
    ov.dwOSVersionInfoSize = sizeof(ov);
    GetVersionEx(&ov);
	switch(ov.dwPlatformId)
	{
	default:
	case VER_PLATFORM_WIN32_WINDOWS:
	case VER_PLATFORM_WIN32s:
		return false;
	case VER_PLATFORM_WIN32_NT:
		return ov.dwMajorVersion>4;
	}
}

/*	BOOL ShellNotifyIconEx(DWORD action,HWND wnd,UINT id,UINT callbackmsg,HICON icon,const char * tip,const char * balloon_title,const char * balloon_msg, UINT timeout)
	{

		NOTIFYICONDATAW nid_new;
		if (is_win2k_or_newer())
		{
			memset(&nid_new,0,sizeof(nid_new));
			nid_new.cbSize = sizeof(nid_new);
			nid_new.hWnd = wnd;
			nid_new.uID = id;
			nid_new.uFlags = 0;
			if (callbackmsg)
			{
				nid_new.uFlags |= NIF_MESSAGE;
				nid_new.uCallbackMessage = callbackmsg;
			}
			if (icon)
			{
				nid_new.uFlags |= NIF_ICON;
				nid_new.hIcon = icon;
			}			
			if (tip)
			{
				nid_new.uFlags |= NIF_TIP;
				wcsncpy(nid_new.szTip,pfc::stringcvt::string_wide_from_utf8(tip),tabsize(nid_new.szTip)-1);
			}

		nid_new.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
		//if (balloon_title || balloon_msg)
		{
			nid_new.uFlags |= NIF_INFO;
			nid_new.uTimeout = timeout;
			if (balloon_title) wcsncpy(nid_new.szInfoTitle,pfc::stringcvt::string_wide_from_utf8(balloon_title),tabsize(nid_new.szInfoTitle)-1);
			if (balloon_msg) wcsncpy(nid_new.szInfo,pfc::stringcvt::string_wide_from_utf8(balloon_msg),tabsize(nid_new.szInfo)-1);	
		}

		return Shell_NotifyIconW(action, &nid_new);
		}
		return FALSE;

	}*/





bool is_rect_null(const RECT * r)
{
	return r->right <= r->left || r->bottom <= r->top;
}

int get_text_width(HDC dc,const char * src,int len)
{
	if (!dc) return 0;
	if (len<=0) return 0;
	else
	{
		SIZE s;
		uGetTextExtentPoint32(dc,src,len,&s);
		return s.cx;
	}
}

//GetTextExtentPoint32 wrapper, removes color marks
int get_text_width_color(HDC dc,const char * src,int len)
{
	if (!dc) return 0;
	int ptr = 0;
	int start = 0;
	int rv = 0;
	if (len<0) len = strlen(src);
	while(ptr<len)
	{
		if (src[ptr]==3)
		{
			rv += get_text_width(dc,src+start,ptr-start);
			ptr++;
			while(ptr<len && src[ptr]!=3) ptr++;
			if (ptr<len) ptr++;
			start = ptr;
		}
		else ptr++;
	}
	rv += get_text_width(dc,src+start,ptr-start);
	return rv;
}

/*int get_text_max_chars(HDC dc,const char * src,int len, unsigned width, unsigned & new_width)
{
	if (!dc) return 0;
	int ptr = 0;
	int start = 0;
	int rv = 0;
	if (len<0) len = strlen(src);
	int used = 0;
	while(ptr<len && used<width)
	{
		if (src[ptr]==3)
		{
	//		rv += get_text_width(dc,src+start,ptr-start);
			ptr++;
			while(ptr<len && src[ptr]!=3) ptr++;
			if (ptr<len) ptr++;
		//	start = ptr;
		}
		int cw = get_text_width(dc,src+ptr,1);
		if (used+cw < width) {rv++;used+=cw;}
		ptr++;
	}
	new_width = used;
//	rv += get_text_width(dc,src+start,ptr-start);

	return (rv != len ? rv : -1);
}*/

int strlen_utf8_colour(const char * src,int len=-1)
{
	int ptr = 0;
	int start = 0;
	int rv = 0;
	if (len<0) len = strlen(src);
	while(ptr<len)
	{
		if (src[ptr]==3)
		{
			rv += pfc::strlen_utf8(src+start,ptr-start);
			ptr++;
			while(ptr<len && src[ptr]!=3) ptr++;
			if (ptr<len) ptr++;
			start = ptr;
		}
		else ptr++;
	}
	rv += pfc::strlen_utf8(src+start,ptr-start);
	return rv;
}


static BOOL text_out_colours_ellipsis(HDC dc,const char * src_c,int len,int x_offset,int pos_y,const RECT * base_clip,bool selected,bool show_ellipsis,DWORD default_color,alignment align)
{

	pfc::string8_fast_aggressive src_s;

	const char * src = src_c;

//	src_s.set_string(src_c,len);

	if (!base_clip) return FALSE;
	
	if (base_clip)
	{
		if (is_rect_null(base_clip) || base_clip->right<=base_clip->left+x_offset || base_clip->bottom<=pos_y) return TRUE;
	}

//	bool need_ellipsis = false;

	RECT clip = *base_clip, ellipsis = {0,0,0,0};

	SetTextAlign(dc,TA_LEFT);
	SetBkMode(dc,TRANSPARENT);
	SetTextColor(dc, default_color);
	
	int height = uGetTextHeight(dc);
	
	int position = clip.left;//item.left+BORDER;

//	if (!need_ellipsis)
	{
		if (align == ALIGN_CENTRE)
		{
			int width = get_text_width_color(dc, src, len);
			position += (clip.right - clip.left - width)/2;
		}
		else if (align == ALIGN_RIGHT)
		{
	/*		if (show_ellipsis)
			{
				int width = get_text_width_color(dc, src, len);
				if (width > clip.right - clip.left - x_offset) 
				{
					int ellipsis_width = get_text_width(dc, ELLIPSIS, ELLIPSIS_LEN)+2;
					if (ellipsis_width <= clip.right - clip.left - x_offset)
					{
						unsigned u_chars = strlen_utf8_colour(src, len);

						INT max_chars = len;

						pfc::array_t<INT> widths(u_chars);

						unsigned trunc_width;
						SIZE s;
						uGetTextExtentExPoint(dc, src, len, clip.right - clip.left - x_offset - ellipsis_width, &max_chars, widths.get_ptr(), &s, trunc_width, false);


						const char * ptr = src;
						if (len) ptr += len-1;
						const char * start = ptr;

						unsigned n = ptr-src;

						int av_width = width - ellipsis_width;

						while(av_width > 0 && ptr > (src) && n)
						{
							if (*ptr==3)
							{
								ptr--;
								while(ptr > src && *ptr!=3) ptr--;
								if (ptr > src && *ptr==3) ptr--;
							}
							else 
							{
								if (n>0)
								av_width -= widths[n]-widths[n-1];
								if (av_width <= 0)
								{
									break;
								}
								n--;
								ptr--;
							}
						}

						src_s.prealloc(len-n);
						src_s += ELLIPSIS;
						src_s.add_string(ptr, start-ptr);

						src = src_s;

						len = src_s.length();
					}
				}
			}*/
			int width = get_text_width_color(dc, src, len);
			position += (clip.right - clip.left - width - x_offset);
		}
		else 
		{ 
			position+=x_offset;
			if (show_ellipsis)
			{
				int width = get_text_width_color(dc, src, len);
				if (width > clip.right - clip.left - x_offset) 
				{
					int ellipsis_width = get_text_width(dc, ELLIPSIS, ELLIPSIS_LEN)+2;
					if (ellipsis_width <= clip.right - clip.left - x_offset)
					{
						INT max_chars = len;
						unsigned trunc_width;
						SIZE s;
						uGetTextExtentExPoint(dc, src, len, clip.right - clip.left - x_offset - ellipsis_width, &max_chars, 0, &s, trunc_width);

						if (max_chars > 0)
						{
							{
								const char * ptr = src;
								const char * start = src;
								ptr += max_chars-1;
					//			while (ptr>start && (*ptr == ' ' || *ptr == '.')) {ptr --;max_chars--;}
							}
						}

						const char * ptr = src;
						const char * start = ptr;

						unsigned n = 0;

						

						while(*ptr && n < max_chars)
						{
							if (*ptr==3)
							{
								ptr++;
								while(*ptr && *ptr!=3) ptr++;
								if (*ptr==3) ptr++;
							}
							else {n++;ptr++;}
						}
						src_s.set_string(src, len);

						src_s.truncate(ptr-start);
						src_s += ELLIPSIS;
						src = src_s;

						len = src_s.length();

					//	need_ellipsis = true;
				/*		ellipsis = clip;
						clip.right = clip.left + x_offset + trunc_width;//-=ellipsis_width;
						ellipsis.left = clip.right;*/
					}
				}
			}
		}
	}

	const char * p_start = src;
	const char * p_block_start = src;
	const char * p_current = src;

	while ((p_current - p_start) < len)
	{
		p_block_start = p_current;
		while ( (p_current - p_start) < len && p_current[0] != 3 )
		{
			/*const char * p_next = uCharNext(p_current);
			if (p_current == p_next)
				break;*/
			p_current++;// = p_next;
		}
		if (p_current > p_block_start)
		{
			int width = get_text_width(dc,p_block_start,p_current-p_block_start);
			uExtTextOut(dc,position,pos_y,ETO_CLIPPED,&clip,p_block_start,p_current-p_block_start,0);
			position += width;
		}
		else if ((p_current - p_start) >= len 
			|| p_current[0] != 3) //this one shouln't happen
			break; 
		if ((p_current - p_start) + 1 < len) //need another at least char for valid colour code operation
		{
			if (p_current[0] == 3)
			{
				COLORREF new_colour;
				COLORREF new_colour_selected;

				bool have_selected = false;

				if (p_current[1]==3) 
				{
					new_colour=new_colour_selected=default_color;
					have_selected=true;
					p_current+=2;
				}
				else
				{
					p_current++;
					new_colour = strtoul_n(p_current,min(6,len-(p_current-p_start)));
					while((p_current-p_start)<len && p_current[0]!=3 && p_current[0]!='|')
					{
						p_current++;
					}
					if ((p_current-p_start)<len && p_current[0]=='|')
					{
						if ((p_current-p_start)+1<len)
						{
							p_current++;
							new_colour_selected = strtoul_n(p_current,min(6,len-(p_current-p_start)));
							have_selected = true;
						}
						else
							break; //unexpected eos, malformed string

						while((p_current-p_start)<len && p_current[0]!=3) p_current++;
					}
					if ((p_current-p_start+1)<len) //need at least one char after colour code
						p_current++;
					else
						break; //no more text, dont need to set colour
				}
				if (selected) new_colour = have_selected ? new_colour_selected : 0xFFFFFF - new_colour;
				SetTextColor(dc,new_colour);
			}
		}
		else
			break; //unexpected eos, malformed string
	}

/*	if (need_ellipsis)
	{
		SetTextColor(dc,default_color);
		uExtTextOut(dc,ellipsis.left+1,pos_y,ETO_CLIPPED,&ellipsis,ELLIPSIS,ELLIPSIS_LEN,0);
	}*/

	return TRUE;
}


BOOL text_out_colours_tab(HDC dc,const char * display,int display_len,int left_offset,int border,const RECT * base_clip,bool selected,DWORD default_color,bool columns,bool use_tab,bool show_ellipsis,alignment align)
{

	if (!base_clip) return FALSE;

	RECT clip = *base_clip;

	if (is_rect_null(&clip)) return TRUE;

	int extra = clip.bottom-clip.top - uGetTextHeight(dc);

	int pos_y = clip.top + (extra/2);
	
	int n_tabs = 0;
	int total_width = 0;
	
	if (use_tab)
	{
		int start = 0;
		int n;
		for(n=0;n<display_len;n++)
		{
			if (display[n]=='\t')
			{
				if (start<n) total_width += get_text_width_color(dc,display+start,n-start) + 2*border;
				start = n+1;
				n_tabs++;
			}
		}
		if (start<display_len)
		{
			//add width of rest of text after last tab
			total_width += get_text_width_color(dc,display+start,display_len-start) + 2*border;
		}
	}

	if (n_tabs == 0)
	{
		clip.left += border;
		clip.right -= border;
		return text_out_colours_ellipsis(dc, display, display_len, left_offset,pos_y,&clip,selected,show_ellipsis,default_color,align);
	}

	
	int tab_total = clip.right - clip.left;
	if (!columns) tab_total -= total_width;
	int ptr = display_len;
	int tab_ptr = 0;
	int written = 0;
	int clip_x = clip.right;
	do
	{
		int ptr_end = ptr;
		while(ptr>0 && display[ptr-1]!='\t') ptr--;
		const char * t_string = display + ptr;
		int t_length = ptr_end - ptr;
		if (t_length>0)
		{
			int t_width = get_text_width_color(dc,t_string,t_length) + border*2 + left_offset;

			int pos_x;
			int pos_x_right;
			
			if (!columns)
			{
				pos_x_right = clip.right - MulDiv(tab_ptr,tab_total,n_tabs) - written/* - left_offset*/;
			}
			else
			{
				if (tab_ptr==0) pos_x_right = clip.right/* - left_offset*/;
				else pos_x_right = clip.right - MulDiv(tab_ptr,tab_total,n_tabs) + t_width/* - left_offset*/;
			}

			if (ptr==0) 
			{
				pos_x = left_offset + border;
			}
			else
			{			
				pos_x = pos_x_right - t_width - clip.left + left_offset + border;
	//			if (pos_x<0) pos_x = clip.left+left_offset;
			}
			
			RECT t_clip = clip;

			//console::printf(" tab %u, right %i   clip %i",tab_ptr,t_clip.right,clip_x);

			if (t_clip.right > clip_x/* + clip.left*/) 
				t_clip.right = clip_x/* + clip.left*/;

			text_out_colours_ellipsis(dc,t_string,t_length,pos_x+border,pos_y,&t_clip,selected,show_ellipsis,default_color,ALIGN_LEFT);

			if (clip_x> (pos_x + clip.left - left_offset-border*2))
				clip_x = pos_x + clip.left - left_offset-border*2;
			
			written += t_width;
		}
		
		if (ptr>0)
		{
			ptr--;//tab char
			tab_ptr++;
		}
	}
	while(ptr>0);
	
	return TRUE;
}



#endif