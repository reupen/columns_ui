#pragma once

namespace mmh
{

	namespace fb2k 
	{

//! Helper implementation.
//! IMPLEMENTATION WARNING: If process being completed creates a window taking caller's window as parent, you must not destroy the parent window inside on_task_completion(). If you need to do so, use PostMessage() or main_thread_callback to delay the deletion.
template<typename t_receiver>
class completion_notify_copyptr : public completion_notify_orphanable {
public:
	void on_completion(unsigned p_code) {
		if (m_receiver != NULL) {
			m_receiver->on_task_completion(m_taskid,p_code);
		}
	}
	void setup(t_receiver p_receiver, unsigned p_task_id) {m_receiver = p_receiver; m_taskid = p_task_id;}
	void orphan() {m_receiver = NULL; m_taskid = 0;}
private:
	t_receiver m_receiver;
	unsigned m_taskid;
};

template<typename t_receiver>
service_ptr_t<completion_notify_orphanable> completion_notify_create_copyptr(t_receiver p_receiver,unsigned p_taskid) {
	service_ptr_t<completion_notify_copyptr<t_receiver> > instance = new service_impl_t<completion_notify_copyptr<t_receiver> >();
	instance->setup(p_receiver,p_taskid);
	return instance;
}

	}
}