#ifndef _MMH_LIBRARY_H_
#define _MMH_LIBRARY_H_

namespace mmh
{
	namespace fb2k
	{
		class library_callback_t
		{
		public:
			//! Called when new items are added to the Media Library.
			virtual void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
			//! Called when some items have been removed from the Media Library.
			virtual void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
			//! Called when some items in the Media Library have been modified.
			virtual void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
		};
		class library_callback_autoreg_t : public library_callback_t
		{
		public:
			library_callback_autoreg_t();
			~library_callback_autoreg_t();
		};
		namespace library_callback_manager
		{
			void g_register_callback (library_callback_t * p_callback);
			void g_deregister_callback (library_callback_t * p_callback);
		};
	}
};

#endif //_MMH_LIBRARY_H_