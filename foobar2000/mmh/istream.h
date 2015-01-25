#pragma once

namespace mmh
{
namespace win32
{
template<class TBase = IStream>
class IStream_memblock_t : public TBase
{
	long refcount;
	pfc::array_t<t_uint8> m_data;
	t_size m_position;
public:

	//const void * get_ptr() {return m_data;} const;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void ** ppvObject)
	{
		if (ppvObject==0) return E_NOINTERFACE;
		else if (iid == IID_IUnknown) {AddRef();*ppvObject = (IUnknown*)this;return S_OK;}
		else if (iid == IID_IStream) {AddRef();*ppvObject = (IStream*)this;return S_OK;}
		else if (iid == IID_ISequentialStream) {AddRef();*ppvObject = (ISequentialStream*)this;return S_OK;}
		else return E_NOINTERFACE;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef() {return InterlockedIncrement(&refcount);}
	virtual ULONG STDMETHODCALLTYPE Release()
	{
		LONG rv = InterlockedDecrement(&refcount);
		if (!rv) delete this;
		return rv;
	}

	virtual HRESULT STDMETHODCALLTYPE Seek( 
		LARGE_INTEGER dlibMove,
		DWORD dwOrigin,
		ULARGE_INTEGER *plibNewPosition)
	{
		if (dwOrigin == STREAM_SEEK_CUR)
		{
			if (dlibMove.QuadPart + (LONGLONG)m_position < 0
				|| dlibMove.QuadPart + (LONGLONG)m_position > (LONGLONG)m_data.get_size())
				return STG_E_INVALIDFUNCTION;
			m_position += (t_size)dlibMove.QuadPart; //Cast should be OK by if condition
		}
		else if (dwOrigin == STREAM_SEEK_END)
		{
			if (dlibMove.QuadPart + (LONGLONG)m_data.get_size() < 0)
				return STG_E_INVALIDFUNCTION;
			m_position = m_data.get_size() - (t_size)dlibMove.QuadPart; //Cast should be OK by if condition
		}
		else if (dwOrigin == STREAM_SEEK_SET)
		{
			if ((ULONGLONG)dlibMove.QuadPart > m_data.get_size())
				return STG_E_INVALIDFUNCTION;
			m_position = (t_size)dlibMove.QuadPart; //Cast should be OK by if condition
		}
		else
			return STG_E_INVALIDFUNCTION;
		if (plibNewPosition)
			plibNewPosition->QuadPart = m_position;
		return S_OK;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Read( 
		/* [length_is][size_is][out] */ void *pv,
		/* [in] */ ULONG cb,
		/* [out] */ ULONG *pcbRead)
	{
		t_size read = min (cb, m_data.get_size() - m_position);
		memcpy(pv, &m_data.get_ptr()[m_position], read);
		m_position += read;
		if (pcbRead)
			*pcbRead = read;
		return S_OK;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Write( 
		/* [size_is][in] */ const void *pv,
		/* [in] */ ULONG cb,
		/* [out] */ ULONG *pcbWritten)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE SetSize( 
		ULARGE_INTEGER libNewSize)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE CopyTo( 
		/* [unique][in] */ IStream *pstm,
		/* [in] */ ULARGE_INTEGER cb,
		/* [out] */ ULARGE_INTEGER *pcbRead,
		/* [out] */ ULARGE_INTEGER *pcbWritten)
	{
		if (cb.QuadPart > m_data.get_size() - m_position)
			return STG_E_INVALIDFUNCTION;
		t_size read = min ((t_size)cb.QuadPart, m_data.get_size() - m_position);
		ULONG pwritten = NULL;
		pstm->Write(&m_data.get_ptr()[m_position], read, &pwritten);
		m_position += read;
		if (pcbRead)
			pcbRead->QuadPart = read;
		if (pcbWritten)
			pcbWritten->QuadPart = pwritten;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Commit( 
		/* [in] */ DWORD grfCommitFlags)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Revert( void)
	{
		return E_FAIL;
	}

	virtual HRESULT STDMETHODCALLTYPE LockRegion( 
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Stat( 
		/* [out] */ __RPC__out STATSTG *pstatstg,
		/* [in] */ DWORD grfStatFlag)
	{
		memset(pstatstg, 0, sizeof(STATSTG));
		pstatstg->cbSize.QuadPart = m_data.get_size();
		pstatstg->type = STGTY_STREAM;
		pstatstg->pwcsName = NULL;
		/*if (!(grfStatFlag & STATFLAG_NONAME))
		{
			if (pstatstg->pwcsName = (LPOLESTR)CoTaskMemAlloc(1*2))
				wcscpy_s(pstatstg->pwcsName, 5, L"AB.jpg");
		}*/
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Clone( 
		/* [out] */ __RPC__deref_out_opt IStream **ppstm)
	{
		*ppstm = new IStream_memblock(m_data.get_ptr(), m_data.get_size());
		return S_OK;
	}

	IStream_memblock_t(const t_uint8 * p_data, t_size size) : refcount(0), m_position(0) {m_data.append_fromptr(p_data, size);};
};
typedef IStream_memblock_t<> IStream_memblock;
};
};