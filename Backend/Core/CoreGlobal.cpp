#include "Core/CoreGlobal.h"

#include "Core/Logger.h"

// 공통 객체의 소유권과 생성·소멸 순서는 이 클래스에서 관리한다.
class CoreGlobal final
{
public:
	CoreGlobal() = default;
	~CoreGlobal() = default;

	CoreGlobal(const CoreGlobal&) = delete;
	CoreGlobal& operator=(const CoreGlobal&) = delete;
	CoreGlobal(CoreGlobal&&) = delete;
	CoreGlobal& operator=(CoreGlobal&&) = delete;

	Logger& GetLogger() noexcept
	{
		return _logger;
	}

private:
	// 멤버는 선언 순서대로 생성되고 역순으로 소멸한다.
	Logger _logger;
};

// 공통 객체를 한 인스턴스에 묶어 수명을 자동 관리한다.
static CoreGlobal coreGlobal;

// 소유권은 CoreGlobal에 있고 외부에는 접근용 포인터만 제공한다.
Logger* const		GLogger = &coreGlobal.GetLogger();
