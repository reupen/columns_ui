#include "../../pfc/pfc.h"

namespace mmh
{
	namespace hash
	{
		enum {sha1_digestsize = 20, sha1_blocksize = 64};

		class sha1_generator
		{
		public:
			sha1_generator();
			void reset();
			void add_data(const void * p_data, t_size p_data_size);
			void finialise(void * p_out);

			static void g_run (const void * p_data, t_size p_data_size, void * p_out);
		private:
			void update_context();

			unsigned h0;
			unsigned h1;
			unsigned h2;
			unsigned h3;
			unsigned h4;
			t_uint64 m_size_data_total;
			pfc::array_staticsize_t<t_uint32> w;
		};

		void sha1_hmac(t_uint8 * key, t_size key_size, t_uint8 * msg, t_size msg_size, void * p_out);
		void sha1(const t_uint8 * p_data, t_uint64 p_data_size, void * p_out);
	};
};