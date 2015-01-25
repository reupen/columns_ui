namespace pfc {
	void base64_encode(pfc::string_base & out, const void * in, t_size inSize);
	void base64_encode_append(pfc::string_base & out, const void * in, t_size inSize);
	t_size base64_decode_estimate(const char * text);
	void base64_decode(const char * text, void * out);
}
