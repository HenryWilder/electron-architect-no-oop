#pragma once
#include <concepts>

template<typename T, size_t _MAX_ITEMS> struct List
{
	static constexpr size_t MAX_ITEMS = _MAX_ITEMS;
	T items[MAX_ITEMS] = {};
	size_t num = 0;
};

template<typename T, size_t _MAX_ITEMS> void Push(List<T, _MAX_ITEMS>& list, const T& item)
{
	if (list.num == list.MAX_ITEMS)
	{
		throw "Tried to push when full";
	}
	list.items[list.num++] = item;
}

template<typename T, size_t _MAX_ITEMS> T Pop(List<T, _MAX_ITEMS>& list)
{
	if (list.num == 0)
	{
		throw "Tried to pop when empty";
	}
	return list.items[--list.num];
}

template<typename Func, typename T> concept is_test_function = requires(Func lambda, T a)
{
	{ lambda(a) } -> std::same_as<bool>;
};

// Returns `list.num` if not found
template<auto test, typename T, size_t _MAX_ITEMS> size_t Find(List<T, _MAX_ITEMS>& list, const auto& cap)
{
	for (size_t i = 0; i < list.num; ++i)
	{
		const T& item = list.items[i];
		if (test(item, cap))
		{
			return i;
		}
	}
	return list.num;
}

template<std::equality_comparable T, size_t _MAX_ITEMS> size_t FindItem(List<T, _MAX_ITEMS>& list, const T& lookingFor)
{
	struct { int lookingFor; } cap{ lookingFor };
	return Find<[](const T& item, const auto& cap) { return item == cap.lookingFor; }>(list, cap);
}
