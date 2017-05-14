#pragma once

/**
 * \brief Implementation of config_object for integers using std::atomic<> to store values in memory.
 * \tparam int_type			Type of the integer to store
 * \tparam memory_order_	std::memory_order enum value
 */
template<typename int_type, std::memory_order memory_order_ = std::memory_order_relaxed>
class ConfigObjectIntegral : public config_object, cfg_var {
public:
	static_assert(std::is_integral<int_type>::value, "int_type must be an integral type.");
	using Type = ConfigObjectIntegral<int_type, memory_order_>;

	ConfigObjectIntegral(const GUID & guid, bool default_value) 
	: cfg_var(guid), m_guid(guid), m_value(default_value) {}

	operator int_type() const
	{
		return get_value();
	}
	Type& operator=(int_type new_value)
	{
		set_value(new_value); return *this;
	}

	int_type get_value() const
	{
		return m_value.load(memory_order_);
	}
	void set_value(int_type new_value)
	{
		m_value.store(new_value, memory_order_);
	}
private:
	GUID get_guid() const override
	{
		return m_guid;
	}
	void get_data(stream_writer * p_stream, abort_callback & p_abort) const override
	{
		p_stream->write_lendian_t(get_value(), p_abort);
	}
	void set_data(stream_reader * p_stream, abort_callback & p_abort, bool p_sendnotify = true) override
	{
		int_type new_value;
		p_stream->read_lendian_t(new_value, p_abort);
		set_value(new_value);
		if (p_sendnotify)
			config_object_notify_manager::g_on_changed(this);
	}
	void get_data_raw(stream_writer * p_stream, abort_callback & p_abort) override
	{
		get_data(p_stream, p_abort);
	}
	void set_data_raw(stream_reader * p_stream, t_size p_sizehint, abort_callback & p_abort) override
	{
		set_data(p_stream, p_abort, false);
	}

	const GUID m_guid;
	std::atomic<int_type> m_value;
};

/**
 * \brief Factory for ConfigObjectIntegral services.
 * \tparam int_type			Type of the integer to store
 * \tparam memory_order_	std::memory_order enum value
 */
template<typename int_type, std::memory_order memory_order_ = std::memory_order_relaxed>
class ConfigObjectIntegralFactory: public service_factory_single_transparent_t<ConfigObjectIntegral<int_type, memory_order_>> {
public:
	using service_factory_single_transparent_t<ConfigObjectIntegral<int_type, memory_order_>>::service_factory_single_transparent_t;
	using ConfigObjectIntegral<int_type, memory_order_>::operator =;
};

/**
* \brief Factory for boolean ConfigObjectIntegral services.
* \tparam memory_order_		std::memory_order enum value
*/
template<std::memory_order memory_order_ = std::memory_order_relaxed>
using ConfigObjectBoolFactory = ConfigObjectIntegralFactory<bool, memory_order_>;
