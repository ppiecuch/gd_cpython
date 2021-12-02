/**
* @file handle_map.h
* @author Jeff Kiah
* @copyright The MIT License (MIT), Copyright (c) 2015 Jeff Kiah
*/
#pragma once
#ifndef HANDLE_MAP_H_
#define HANDLE_MAP_H_

#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <exception>
#include <type_traits>

#if !defined(_NOEXCEPT)
# if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026
#  define _NOEXCEPT noexcept
# elif __GNUC__ >= 4
#  define _NOEXCEPT _GLIBCXX_USE_NOEXCEPT
# else
#  define _NOEXCEPT
# endif // if __GNUC__ >= 4
#endif  // ifndef _NOEXCEPT

#if __GNUC__ && __GNUC__ < 5
#define IS_TRIVIALLY_COPYABLE(T) __has_trivial_copy(T)
#else
#define IS_TRIVIALLY_COPYABLE(T) std::is_trivially_copyable<T>::value
#endif

/**
* @struct Id_T
* @var	free		0 if active, 1 if slot is part of freelist, only applicable to inner ids
* @var	type_id		relates to m_item_type_id parameter of handle_map
* @var	generation	incrementing generation of data at the index, for tracking accesses to old data
* @var	index		When used as a handle (outer id, given to the client):
*						free==0, index of id in the sparseIds array
*					When used as an inner id (stored in sparseIds array):
*						free==0, index of the item in the dense items array
*						free==1, index of next free slot, forming an embedded linked list
* @var	value		unioned with the above four vars, used for direct comparison of ids
*/
struct Id_T {
	union {
		/**
		* the order of this bitfield is important for sorting prioritized by free, then type_id,
		* then generation, then index
		*/
		struct {
			uint32_t index : 16;
			uint32_t generation : 14;
			uint32_t type_id : 1;
			uint32_t free : 1;
		};
		uint32_t value;
	};
};
typedef std::vector<Id_T> IdSet_T;
#define NullId_T Id_T{}
inline static Id_T make_handle(uint32_t v) { Id_T Id; Id.value = v; return Id; }

/**
* @class handle_map
*	Stores objects using a dense inner array and sparse outer array scheme for good cache coherence
*	of the inner items. The sparse array contains handles (outer ids) used to identify the item,
*	and provides an extra indirection allowing the inner array to move items in memory to keep them
*	tightly packed. The sparse array contains an embedded FIFO freelist, where removed ids push to
*	the back while new ids pop from the front.
*
* @tparam	T	type of item to be stored
*/
template <typename T>
class handle_map {
public:
	/**
	* @struct Meta_T
	*/
	struct Meta_T {
		uint32_t	dense_to_sparse;	//!< index into m_sparse_ids array stored in m_meta
	};

	typedef std::vector<T>      DenseSet_T;
	typedef std::vector<Meta_T> MetaSet_T;

	// Functions

	/**
	* Get a direct reference to a stored item by handle
	* @param[in]	handle		id of the item
	* @returns reference to the item
	*/
	T&			at(Id_T handle);
	const T&	at(Id_T handle) const;
	T&			operator[](Id_T handle)			{ return at(handle); }
	const T&	operator[](Id_T handle) const	{ return at(handle); }

	/**
	* Look for an object and return handle
	* @param[in]	i		object
	* @param[out]	idT		handle of the object
	* @returns true if object has been found
	*/
	bool		find(const T &i, Id_T &id) const;

	/**
	* create one item with default initialization
	* @tparam		Params	initialization arguments passed to constructor of item
	* @returns the id
	*/
	template <typename... Params>
	Id_T emplace(Params... args) { return insert(T{ args... }); }

	/**
	* create n items with initialization args specified by Params, return vector of ids
	* @param[in]	n		number of items to create
	* @tparam		Params	initialization arguments passed to constructor of each item created
	* @returns a collection of ids
	*/
	template <typename... Params>
	IdSet_T emplace_items(int n, Params... args);

	/**
	* iterators over the dense set, they are invalidated by inserting and removing
	*/
	typename DenseSet_T::iterator		begin()			{ return m_items.begin(); }
	typename DenseSet_T::const_iterator	cbegin() const	{ return m_items.cbegin(); }
	typename DenseSet_T::iterator		end()			{ return m_items.end(); }
	typename DenseSet_T::const_iterator	cend() const	{ return m_items.cend(); }

	/**
	* remove the item identified by the provided handle
	* @param[in]	handle		id of the item
	* @returns count of items removed (0 or 1)
	*/
	size_t erase(Id_T handle);

	/**
	* remove the items identified in the set of handles
	* @param[in]	handles		set of ids
	* @returns count of items removed
	*/
	size_t erase_items(const IdSet_T& handles);

	/**
	* add one item, forwarding the provided i into the store, return id
	* @param[in]	i	rvalue ref of of the object to move into inner storage
	* @returns the id
	*/
	Id_T insert(T&& i);

	/**
	* add one item, copying the provided i into the store, return id
	* @param[in]	i	const ref of of the object to copy into inner storage
	* @returns the id
	*/
	Id_T insert(const T& i);

	/**
	* Removes all items, leaving the m_sparse_ids set intact by adding each entry to the free-
	* list and incrementing its generation. This operation is slower than @c reset, but safer
	* for the detection of stale handle lookups later (in debug builds). Prefer to use @c reset
	* if safety is not a concern.
	* Complexity is linear.
	*/
	void clear() _NOEXCEPT;

	/**
	* Removes all items, destroying the m_sparse_ids set. Leaves the container's capacity, but
	* otherwise equivalent to a default-constructed container. This is faster than @c clear,
	* but cannot safely detect lookups by stale handles obtained before the reset. Use @c clear
	* if safety is a concern, at least until it's proven not to be a problem.
	* Complexity is constant.
	*/
	void reset() _NOEXCEPT;

	/**
	* @returns true if handle handle refers to a valid item
	*/
	bool is_valid(Id_T handle) const;

	/**
	* @returns size of the dense items array
	*/
	size_t size() const _NOEXCEPT { return m_items.size(); }

	/**
	* @returns capacity of the dense items array
	*/
	size_t capacity() const _NOEXCEPT { return m_items.capacity(); }

	/**
	* defragment uses the comparison function @c comp to establish an ideal order for the dense
	*	set in order to maximum cache locality for traversals. The dense set can become
	*	fragmented over time due to removal operations. This can be an expensive operation, so
	*	the sort operation is reentrant. Use the @c maxSwaps parameter to limit the number of
	*	swaps that will occur before the function returns.
	* @param[in]	comp	comparison function object, function pointer, or lambda
	* @param[in]	maxSwaps	maximum number of items to reorder in the insertion sort
	*	before the function returns. Pass 0 (default) to run until completion.
	* @tparam	Compare	comparison function object which returns ?true if the first argument is
	*	greater than (i.e. is ordered after) the second. The signature of the comparison
	*	function should be equivalent to the following:
	*	@code bool cmp(const T& a, const T& b); @endcode
	*	The signature does not need to have const &, but the function object must not modify
	*	the objects passed to it.
	* @returns the number of swaps that occurred, keeping in mind that this value could
	*	overflow on very large data sets
	*/
	template <typename Compare>
	size_t	defragment(Compare comp, size_t maxSwaps = 0);


	/**
	* these functions provide direct access to inner arrays, don't add or remove items, just
	* use them for lookups and iterating over the items
	*/
	DenseSet_T&			get_items()					{ return m_items; }
	const DenseSet_T&	get_items() const			{ return m_items; }
	MetaSet_T&			get_meta()					{ return m_meta; }
	const MetaSet_T&	get_meta() const			{ return m_meta; }
	IdSet_T&			get_ids()					{ return m_sparse_ids; }
	const IdSet_T&		get_ids() const				{ return m_sparse_ids; }

	uint32_t			get_free_list_front() const	{ return m_free_list_front; }
	uint32_t			get_free_list_back() const	{ return m_free_list_back; }

	uint16_t			get_item_type_id() const	{ return m_item_type_id; }

	/**
	* @returns index into the inner DenseSet for a given outer id
	*/
	uint32_t			get_inner_index(Id_T handle) const;

	/**
	* Constructor
	* @param	item_type_id		type_id used by the Id_T::type_id variable for this container
	* @param	reserve_count	reserve space for inner storage
	*/
	explicit handle_map(uint16_t item_type_id, size_t reserve_count)
		: m_item_type_id(item_type_id)
	{
		m_sparse_ids.reserve(reserve_count);
		m_items.reserve(reserve_count);
		m_meta.reserve(reserve_count);
	}

private:

	/**
	* free_list is empty when the front is set to 32 bit max value (the back will match)
	* @returns true if empty
	*/
	bool free_list_empty() const { return (m_free_list_front == 0xFFFFFFFF); }

	// Variables

	uint32_t	m_free_list_front = 0xFFFFFFFF; //!< start index in the embedded ComponentId freelist
	uint32_t	m_free_list_back  = 0xFFFFFFFF; //!< last index in the freelist

	uint16_t	m_item_type_id;	//!< the Id_T::type_id to use for ids produced by this handle_map<T>

	uint8_t		m_fragmented = 0; //<! set to 1 if modified by insert or erase since last complete defragment

	IdSet_T		m_sparse_ids;	//!< stores a set of Id_Ts, these are "inner" ids indexing into m_items
	DenseSet_T	m_items;		//!< stores items of type T
	MetaSet_T	m_meta;			//!< stores Meta_T type for each item
};

// inline details

// struct Id_T comparison functions

inline bool operator==(const Id_T& a, const Id_T& b) { return (a.value == b.value); }
inline bool operator!=(const Id_T& a, const Id_T& b) { return (a.value != b.value); }
inline bool operator< (const Id_T& a, const Id_T& b) { return (a.value < b.value); }
inline bool operator> (const Id_T& a, const Id_T& b) { return (a.value > b.value); }

// class handle_map

template <typename T>
Id_T handle_map<T>::insert(T&& i)
{
	Id_T handle;
	m_fragmented = 1;

	if (free_list_empty()) {
		Id_T innerId = {{{
			(uint32_t)m_items.size(),
			1,
			m_item_type_id,
			0
		}}};

		handle = innerId;
		handle.index = (uint32_t)m_sparse_ids.size();

		m_sparse_ids.push_back(innerId);
	} else {
		uint32_t outerIndex = m_free_list_front;
		Id_T &innerId = m_sparse_ids.at(outerIndex);

		m_free_list_front = innerId.index; // the index of a free slot refers to the next free slot
		if (free_list_empty()) {
			m_free_list_back = m_free_list_front;
		}

		// convert the index from freelist to inner index
		innerId.free = 0;
		innerId.index = (uint32_t)m_items.size();

		handle = innerId;
		handle.index = outerIndex;
	}

	m_items.push_back(std::forward<T>(i));
	m_meta.push_back({ handle.index });

	return handle;
}


template <typename T>
Id_T handle_map<T>::insert(const T& i)
{
	return insert(std::move(T{ i }));
}


# if __GNUC__ > 4
template <typename T>
template <typename... Params>
IdSet_T handle_map<T>::emplace_items(int n, Params... args)
{
	IdSet_T handles(n);
	assert(n > 0 && "emplace_items called with n = 0");
	m_fragmented = 1;

	m_items.reserve(m_items.size() + n); // reserve the space we need (if not already there)
	m_meta.reserve(m_meta.size() + n);

	std::generate_n(handles.begin(), n, [&, this](){ return emplace(args...); });

	return handles; // efficient to return vector by value, copy elided with NVRO (or with C++11 move semantics)
}
#endif


template <typename T>
size_t handle_map<T>::erase(Id_T handle)
{
	if (!is_valid(handle)) {
		return 0;
	}
	m_fragmented = 1;

	Id_T innerId = m_sparse_ids[handle.index];
	uint32_t innerIndex = innerId.index;

	// push this slot to the back of the freelist
	innerId.free = 1;
	++innerId.generation; // increment generation so remaining outer ids go stale
	innerId.index = 0xFFFF; // max numeric value represents the end of the freelist
	m_sparse_ids[handle.index] = innerId; // write outer id changes back to the array

	if (free_list_empty()) {
		// if the freelist was empty, it now starts (and ends) at this index
		m_free_list_front = handle.index;
		m_free_list_back = m_free_list_front;
	} else {
		m_sparse_ids[m_free_list_back].index = handle.index; // previous back of the freelist points to new back
		m_free_list_back = handle.index; // new freelist back is stored
	}

	// remove the component by swapping with the last element, then pop_back
	if (innerIndex != m_items.size() - 1) {
		std::swap(m_items.at(innerIndex), m_items.back());
		std::swap(m_meta.at(innerIndex), m_meta.back());

		// fix the ComponentId index of the swapped component
		m_sparse_ids[m_meta.at(innerIndex).dense_to_sparse].index = innerIndex;
	}

	m_items.pop_back();
	m_meta.pop_back();

	return 1;
}


template <typename T>
size_t handle_map<T>::erase_items(const IdSet_T& handles)
{
	size_t count = 0;
	for (auto h : handles) {
		count += erase(h);
	}
	return count;
}


template <typename T>
void handle_map<T>::clear() _NOEXCEPT
{
	uint32_t size = static_cast<uint32_t>(m_sparse_ids.size());

	if (size > 0) {
		m_items.clear();
		m_meta.clear();

		m_free_list_front = 0;
		m_free_list_back = size - 1;
		m_fragmented = 0;

		for (uint32_t i = 0; i < size; ++i) {
			auto& id = m_sparse_ids[i];
			id.free = 1;
			++id.generation;
			id.index = i + 1;
		}
		m_sparse_ids[size - 1].index = 0xFFFFFFFF;
	}
}


template <typename T>
void handle_map<T>::reset() _NOEXCEPT
{
	m_free_list_front = 0xFFFFFFFF;
	m_free_list_back = 0xFFFFFFFF;
	m_fragmented = 0;

	m_items.clear();
	m_meta.clear();
	m_sparse_ids.clear();
}


template <typename T>
inline T& handle_map<T>::at(Id_T handle)
{
	assert(handle.index < m_sparse_ids.size() && "outer index out of range");

	Id_T innerId = m_sparse_ids[handle.index];

	assert(handle.type_id == m_item_type_id && "type_id mismatch");
	assert(handle.generation == innerId.generation && "at called with old generation");
	assert(innerId.index < m_items.size() && "inner index out of range");

	return m_items[innerId.index];
}


template <typename T>
inline const T& handle_map<T>::at(Id_T handle) const
{
	assert(handle.index < m_sparse_ids.size() && "outer index out of range");

	Id_T innerId = m_sparse_ids[handle.index];

	assert(handle.type_id == m_item_type_id && "type_id mismatch");
	assert(handle.generation == innerId.generation && "at called with old generation");
	assert(innerId.index < m_items.size() && "inner index out of range");

	return m_items[innerId.index];
}


template <typename T>
inline bool handle_map<T>::find(const T &i, Id_T &idT) const
{
	auto it = std::find(m_items.begin(), m_items.end(), i);
	if (it != m_items.end()) {
		const int index = it - m_items.begin();
		idT = m_sparse_ids[index];
		return true;
	} else {
		return false;
	}
}


template <typename T>
inline bool handle_map<T>::is_valid(Id_T handle) const
{
	if (handle.index >= m_sparse_ids.size()) {
		return false;
	}

	Id_T innerId = m_sparse_ids[handle.index];

	return (innerId.index < m_items.size() &&
			handle.type_id == m_item_type_id &&
			handle.generation == innerId.generation);
}


template <typename T>
inline uint32_t handle_map<T>::get_inner_index(Id_T handle) const
{
	assert(handle.index < m_sparse_ids.size() && "outer index out of range");

	Id_T innerId = m_sparse_ids[handle.index];

	assert(handle.type_id == m_item_type_id && "type_id mismatch");
	assert(handle.generation == innerId.generation && "at called with old generation");
	assert(innerId.index < m_items.size() && "inner index out of range");

	return innerId.index;
}


template <typename T>
template <typename Compare>
size_t handle_map<T>::defragment(Compare comp, size_t maxSwaps)
{
	if (m_fragmented == 0) { return 0; }
	size_t swaps = 0;

	int i = 1;
	for (; i < m_items.size() && (maxSwaps == 0 || swaps < maxSwaps); ++i) {
		T tmp = m_items[i];
		Meta_T tmpMeta = m_meta[i];

		int j = i - 1;
		int j1 = j + 1;

		// trivially copyable implementation
		if (IS_TRIVIALLY_COPYABLE(T)) {
			while (j >= 0 && comp(m_items[j], tmp)) {
				m_sparse_ids[m_meta[j].dense_to_sparse].index = j1;
				--j;
				--j1;
			}
			if (j1 != i) {
				memmove(&m_items[j1+1], &m_items[j1], sizeof(T) * (i - j1));
				memmove(&m_meta[j1+1], &m_meta[j1], sizeof(Meta_T) * (i - j1));
				++swaps;

				m_items[j1] = tmp;
				m_meta[j1] = tmpMeta;
				m_sparse_ids[m_meta[j1].dense_to_sparse].index = j1;
			}
		} else {
			// standard implementation
			while (j >= 0 && (maxSwaps == 0 || swaps < maxSwaps) &&
					comp(m_items[j], tmp))
			{
				m_items[j1] = std::move(m_items[j]);
				m_meta[j1] = std::move(m_meta[j]);
				m_sparse_ids[m_meta[j1].dense_to_sparse].index = j1;
				--j;
				--j1;
				++swaps;
			}

			if (j1 != i) {
				m_items[j1] = tmp;
				m_meta[j1] = tmpMeta;
				m_sparse_ids[m_meta[j1].dense_to_sparse].index = j1;
			}
		}
	}
	if (i == m_items.size()) {
		m_fragmented = 0;
	}

	return swaps;
}

#endif // HANDLE_MAP_H_
