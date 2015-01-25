#pragma once

namespace mmh
{

template<class T>
class comptr_t
{
public:
	comptr_t(IUnknown * pUnk)
		: m_Unk(NULL)
	{
		copy(pUnk);
	}
	comptr_t()
		: m_Unk(NULL) {};

	inline comptr_t(const comptr_t <T> &p_source)
		 : m_Unk(NULL) 
	{
		copy(p_source);
	}

	~comptr_t()
	{
		release();
	}
	inline void release()
	{
		if (m_Unk)
		{
			m_Unk->Release();
			m_Unk=NULL;
		}
	}
	inline void copy(IUnknown * p_Unk)
	{
		release();
		if (p_Unk)
		{
			p_Unk->QueryInterface(&m_Unk);
		}
	}

	template<class Q>
	inline void copy(const comptr_t<Q> & p_source) {copy(p_source.get_ptr());}

//	template<class Q>
//	inline const comptr_t<T> & operator=(const comptr_t<Q> & p_Unk) {copy(p_Unk); return *this;}

	inline const comptr_t<T> & operator=(const comptr_t<T> & p_Unk) {copy(p_Unk); return *this;}

	inline const comptr_t<T> & operator=(IUnknown * p_Unk) {copy(p_Unk); return *this;}

	inline T* operator->() const {assert(m_Unk);return m_Unk;}

	inline operator void** () {release();return (void**)&m_Unk;}
	inline operator T** () {release();return &m_Unk;}
	T** get_pp() {release(); return &m_Unk;}
	inline operator T* () {
		//assert(m_Unk);  //sometimes you may want to pass a NULL pointer as a function parameter
		return m_Unk;}

	inline T* get_ptr() const {return m_Unk;}
	inline bool is_valid() const {return m_Unk != 0;}
	inline bool is_empty() const {return m_Unk == 0;}

	inline bool operator==(const comptr_t<T> & p_item) const {return m_Unk == p_item.m_Unk;}
	inline bool operator!=(const comptr_t<T> & p_item) const {return m_Unk != p_item.m_Unk;}
	inline bool operator>(const comptr_t<T> & p_item) const {return m_Unk > p_item.m_Unk;}
	inline bool operator<(const comptr_t<T> & p_item) const {return m_Unk < p_item.m_Unk;}

	inline static void g_swap(comptr_t<T> & item1, comptr_t<T> & item2)
	{
		pfc::swap_t(item1.m_Unk,item2.m_Unk);
	}

	HRESULT instantiate(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
	{
		return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), *this);
	}

	HRESULT instantiate();

private:
	T * m_Unk;
};

}