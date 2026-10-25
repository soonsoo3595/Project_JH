#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// 현재는 표준 할당자를 사용한다.
template<typename T, std::size_t Size>
using Array = std::array<T, Size>;

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using List = std::list<T>;

template<typename Key, typename T, typename Compare = std::less<Key>>
using Map = std::map<Key, T, Compare>;

template<typename Key, typename Compare = std::less<Key>>
using Set = std::set<Key, Compare>;

template<typename T>
using Deque = std::deque<T>;

template<typename T, typename Container = Deque<T>>
using Queue = std::queue<T, Container>;

template<typename T, typename Container = Deque<T>>
using Stack = std::stack<T, Container>;

template<typename T, typename Container = Vector<T>,
	typename Compare = std::less<typename Container::value_type>>
using PriorityQueue = std::priority_queue<T, Container, Compare>;

template<typename Key, typename T, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using HashMap = std::unordered_map<Key, T, Hasher, KeyEqual>;

template<typename Key, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using HashSet = std::unordered_set<Key, Hasher, KeyEqual>;

// 일반 텍스트는 UTF-8로 다룬다. 인코딩 변환이나 검증을 수행하는 타입은 아니다.
using String = std::string;
