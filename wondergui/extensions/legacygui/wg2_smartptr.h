#ifndef	WG2_SMARTPTR_DOT_H
#define	WG2_SMARTPTR_DOT_H

//____ WgWeakPtrTarget _________________________________________________________

class WgWeakPtrTarget;
class WgWeakPtrImpl;

class WgWeakPtrHub
{
public:
	int                    refCnt;
	WgWeakPtrTarget *    pObj;
};

class WgWeakPtrTarget
{
	friend class WgWeakPtrImpl;
protected:
	WgWeakPtrTarget() : m_pHub(0) {}
	~WgWeakPtrTarget() { if( m_pHub ) m_pHub->pObj = 0; }

private:
	WgWeakPtrHub *    m_pHub;

};


//____ WgWeakPtrImpl __________________________________________________________

class WgWeakPtrImpl
{
public:
	WgWeakPtrImpl() { m_pHub = 0; }
	WgWeakPtrImpl( WgWeakPtrTarget * pObj );
	~WgWeakPtrImpl();

	void copy( WgWeakPtrImpl const & r );

	WgWeakPtrHub * m_pHub;
};


//____ WgWeakPtr ______________________________________________________________

template<class T> class WgWeakPtr : private WgWeakPtrImpl
{
public:
	WgWeakPtr() {}
	WgWeakPtr( T * pObj ) : WgWeakPtrImpl( pObj ) {}

	WgWeakPtr(const WgWeakPtr<T>& r)
	{

		m_pHub = r.m_pHub;
		if( m_pHub )
			m_pHub->refCnt++;
	}

	~WgWeakPtr() {}


	inline WgWeakPtr<T> & operator=( WgWeakPtr<T> const & r)
	{
		copy( r );
		return *this;
	}

	inline T & operator*() const { return * GetRealPtr(); }
	inline T * operator->() const { return GetRealPtr(); }

	//TODO: Fix so that we get right value if both are null-pointers, but have different hubs.
	inline bool operator==(const WgWeakPtr<T>& other) const { return m_pHub == other.m_pHub; }
	inline bool operator!=(const WgWeakPtr<T>& other) const { return m_pHub != other.m_pHub; }
	inline bool operator<(const WgWeakPtr<T>& other) const { return m_pHub < other.m_pHub ? true : false; }
	inline bool operator>(const WgWeakPtr<T>& other) const { return m_pHub > other.m_pHub ? true : false; }
	inline bool operator<=(const WgWeakPtr<T>& other) const { return m_pHub <= other.m_pHub ? true : false; }
	inline bool operator>=(const WgWeakPtr<T>& other) const { return m_pHub >= other.m_pHub ? true : false; }

	inline operator bool() const { return (m_pHub != 0 && m_pHub->pObj != 0); }

	inline T * GetRealPtr() const
	{
		if( m_pHub && m_pHub->pObj )
			return static_cast<T*>(m_pHub->pObj);
		else
			return reinterpret_cast<T*>(0);
	}
};



#endif //WG2_SMARTPTR_DOT_H

