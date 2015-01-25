#include "stdafx.h"

namespace mmh { namespace fb2k {
	initquit_factory_t<initquit_multiplexer> g_initquit_multiplexer;

	void initquit_multiplexer::on_init() 
	{
		for (t_size i = 0, count = m_instances.get_count(); i<count; i++)
			m_instances[i]->on_init();
	};
	void initquit_multiplexer::on_quit() 
	{
		for (t_size i = 0, count = m_instances.get_count(); i<count; i++)
			m_instances[i]->on_quit();
	};

	void initquit_multiplexer::g_register_instance(initquit_dynamic * ptr)
	{
		g_initquit_multiplexer.get_static_instance().register_instance(ptr);
	}
	void initquit_multiplexer::g_deregister_instance(initquit_dynamic * ptr)
	{
		g_initquit_multiplexer.get_static_instance().deregister_instance(ptr);
	}

} }