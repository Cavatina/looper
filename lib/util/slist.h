#ifndef _CAVATINA_SLIST_H_
#define _CAVATINA_SLIST_H_

namespace cav
{
	template <class T>
	class slist_mrsw
	{
	public:
		typedef slist_mrsw<T> list_type;
		slist_mrsw(T *data_ = 0, list_type *next_ = 0)
			: m_next(next_), m_data(data_) {}
		~slist_mrsw() {
			T *p = m_data;
			m_data = 0;
			delete p;
			list_type t = m_next;
			m_next = 0;
			delete t;
		}
		list_type *cur() { return this; }
		list_type *next() { return m_next; }
		T *data() { return m_data; }

		list_type *push(T *data_){
			if(!m_data){
				m_data = data_;
				return this;
			}
			return new slist_mrsw(data_, this);
		}

	private:
		list_type * volatile m_next;
		T *m_data;
	};
}

#endif
