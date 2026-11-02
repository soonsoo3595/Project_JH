#pragma once

#include <atomic>
#include <climits>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

// 출력·갱신 매개변수 표시용
#ifndef OUT
#define OUT
#endif

// 비트 수가 명확해야 하는 정수에 사용한다.
using BYTE		= unsigned char;
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
using Atomic			= std::atomic<T>;

using Mutex				= std::mutex;
using CondVar			= std::condition_variable;
using UniqueLock		= std::unique_lock<Mutex>;
using LockGuard			= std::lock_guard<Mutex>;

// Ref
using IocpObjectRef		= std::shared_ptr<class IocpObject>;
using IocpCoreRef		= std::shared_ptr<class IocpCore>;
using SessionRef		= std::shared_ptr<class Session>;
using ListenerRef		= std::shared_ptr<class Listener>;
using ServiceRef		= std::shared_ptr<class Service>;
