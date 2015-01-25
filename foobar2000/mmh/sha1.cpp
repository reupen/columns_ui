#include "stdafx.h"

namespace mmh
{

namespace hash
{

#define ROTL(X,n) ( ( ( X ) << n ) | ( ( X ) >> ( 32 - n ) ) )
#define ROTR(X,n) ( ( ( X ) >> n ) | ( ( X ) << ( 32 - n ) ) )



		sha1_generator::sha1_generator() : w(80)
		{
			reset();
		}

		void sha1_generator::g_run (const void * p_data, t_size p_data_size, void * p_out)
		{
			sha1_generator sha1;
			sha1.add_data(p_data, p_data_size);
			sha1.finialise(p_out);
		}

		void sha1_generator::reset()
		{
			h0 = 0x67452301;
			h1 = 0xEFCDAB89;
			h2 = 0x98BADCFE;
			h3 = 0x10325476;
			h4 = 0xC3D2E1F0;
			m_size_data_total = 0;
		}
		void sha1_generator::add_data(const void * p_data_raw, t_size p_data_size)
		{
			t_size w_position = (m_size_data_total%64);
			t_uint8 * w8 = (t_uint8*)w.get_ptr();
			const t_uint8 * p_data = (const t_uint8*)p_data_raw;

			for (t_size i = 0; i < p_data_size; i++)
			{
				//meh endianness
				t_size index = w_position - (w_position%4) + 3 - (w_position%4);
				w8[index] = p_data[i];
				if (++w_position == 64) {update_context(); w_position = 0;}
			}

			m_size_data_total += p_data_size;
		}

		void sha1_generator::finialise(void * p_out)
		{
			{
				pfc::array_t<t_uint8> padding;

				t_size append = 0, mod = (m_size_data_total + 1) % 64;
				if (mod > 56)
					append = 64-(mod-56);
				else if (mod < 56)
					append = 56-mod;

				padding.set_size (append + 1);
				padding[0] = 0x80;
				{
					for (t_size i=0; i<append; i++)
						padding[i+1] = 0;
				}

				t_uint64 sizebe = m_size_data_total*8;

				byte_order::order_native_to_be_t(sizebe);
				padding.append_fromptr((t_uint8*)(&sizebe), sizeof(sizebe));
				
				add_data(padding.get_ptr(), padding.get_size());
			}
			t_uint32 * out = (t_uint32*)p_out;

			byte_order::order_native_to_be_t(h0);
			byte_order::order_native_to_be_t(h1);
			byte_order::order_native_to_be_t(h2);
			byte_order::order_native_to_be_t(h3);
			byte_order::order_native_to_be_t(h4);
			out[0] = h0;
			out[1] = h1;
			out[2] = h2;
			out[3] = h3;
			out[4] = h4;

			reset();
		}
	
	void sha1_generator::update_context()
		{
			for (t_size j=16; j<80; j++)
			{
				w[j] = ROTL( (w[j-3] ^ w[j-8] ^ w[j-14] ^ w[j-16]), 1);
			}

			t_uint32 a = h0;
			t_uint32 b = h1;
			t_uint32 c = h2;
			t_uint32 d = h3;
			t_uint32 e = h4;

			for (t_size j=0; j<80; j++)
			{
				t_uint32 f=0, k=0;
				if (j <= 19)
				{
					f = (b & c) | ((~ b) & d);
					k = 0x5A827999;
				}
				else if (j <= 39)
				{
					f = b ^ c ^ d;
					k = 0x6ED9EBA1;
				}
				else if (j <= 59)
				{
					f = (b & c) | (b & d) | (c & d);
					k = 0x8F1BBCDC;
				}
				else
				{
					f = b ^ c ^ d;
					k = 0xCA62C1D6;
				}


				t_uint32 temp = (ROTL(a,5)) + f + e + k + w[j];
				e = d;
				d = c;
				c = ROTL(b, 30);
				b = a;
				a = temp;
			}

			h0 = h0 + a;
			h1 = h1 + b;
			h2 = h2 + c;
			h3 = h3 + d;
			h4 = h4 + e;
		}

void sha1_hmac(t_uint8 * key, t_size key_size, t_uint8 * msg, t_size msg_size, void * p_out)
{
	pfc::array_t<t_uint8> opad, ipad, s1;
	opad.set_size(sha1_blocksize);
	ipad.set_size(sha1_blocksize);
	s1.set_size(sha1_digestsize);
	opad.fill(0x5c);
	ipad.fill(0x36);

	pfc::array_t<t_uint8> key2;

    if (key_size > sha1_blocksize)
	{
		key2.set_size(sha1_digestsize);
        sha1(key, key_size, key2.get_ptr());
	}
	else
		key2.append_fromptr(key, key_size);

	t_size i;
	t_size key2_size = key2.get_size();
    for (i=0; i<key2_size; i++)
	{
        ipad[i] = ipad[i] ^ key2[i];
        opad[i] = opad[i] ^ key2[i];
	}
	sha1_generator sha1_i, sha1_o;
	sha1_i.add_data(ipad.get_ptr(), ipad.get_size());
	sha1_i.add_data(msg, msg_size);
	sha1_i.finialise(s1.get_ptr());

#if 0
	pfc::array_t<t_uint8> buffer;
	buffer.append(ipad);
	buffer.append_fromptr(msg, msg_size);
	sha1(buffer.get_ptr(), buffer.get_size(), s1.get_ptr());

	buffer.set_size(0);
	buffer.append(opad);
	buffer.append(s1);

	sha1(buffer.get_ptr(), buffer.get_size(), p_out);
#endif

	sha1_o.add_data(opad.get_ptr(), opad.get_size());
	sha1_o.add_data(s1.get_ptr(), s1.get_size());
	sha1_o.finialise(p_out);
}

	void sha1(const t_uint8 * p_data, t_uint64 p_data_size, void * p_out)
	{
		sha1_generator::g_run(p_data, p_data_size, p_out);
	}
}
}