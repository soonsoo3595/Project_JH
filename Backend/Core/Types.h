#pragma once

#include <atomic>
#include <climits>
#include <condition_variable>
#include <cstdint>
#include <mutex>

// 출력·갱신 매개변수 표시용이다. Windows SDK에 정의되어 있으면 그대로 사용한다.
#ifndef OUT
#define OUT
#endif

// 비트 수가 명확해야 하는 정수에 사용한다.
using int8		= std::int8_t;
using int16		= std::int16_t;
using int32		= std::int32_t;
using int64		= std::int64_t;

using uint8		= std::uint8_t;
using uint16	= std::uint16_t;
using uint32	= std::uint32_t;
using uint64	= std::uint64_t;

using float32	= float;
using float64	= double;

static_assert(sizeof(float32) * CHAR_BIT == 32, "float32는 32비트여야 합니다.");
static_assert(sizeof(float64) * CHAR_BIT == 64, "float64는 64비트여야 합니다.");

template<typename T>
using Atomic		= std::atomic<T>;

using Mutex			= std::mutex;
using CondVar		= std::condition_variable;
using UniqueLock	= std::unique_lock<Mutex>;
using LockGuard		= std::lock_guard<Mutex>;

// bool·char·std::size_t와 문자열·포인터는 원래 타입을 사용한다.
// 별칭은 새 타입이 아니며 네트워크 전송 시 바이트 순서·직렬화는 별도로 처리한다.
