#ifndef _COLUMNS_CALLBACK_H_
#define _COLUMNS_CALLBACK_H_

#if 0

class switcher_callback
{
public:
	virtual void on_playlist_switch_before(unsigned from,unsigned to)=0;
	virtual void on_playlist_switch_after(unsigned from,unsigned to)=0;

	virtual void on_reorder(const int * order,unsigned count)=0;
	virtual void on_new_playlist(const char * name,unsigned idx,const ptr_list_interface<metadb_handle> & data)=0;
	virtual void on_delete_playlist(unsigned idx)=0;
	virtual void on_rename_playlist(unsigned idx,const char * new_name)=0;

	virtual void on_item_replaced(unsigned pls,unsigned item,metadb_handle * from,metadb_handle * to)=0;
};
#endif

#endif