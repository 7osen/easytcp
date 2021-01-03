#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_
#include <condition_variable>
#include <mutex>
class CellSemaphore
{
public:
	CellSemaphore()
	{

	}
	virtual ~CellSemaphore()
	{

	}

private:

	std::condition_variable _cv;

	std::mutex _mutex;

	int _wait = 0;

	int _wakeup = 0;
protected:
	void Wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			_cv.wait(lock, [this]()->bool {
				return _wakeup > 0;
				});//falseÊ±×èÈû
			--_wakeup;
		}
	}

	void Wakeup()
	{
		if (++_wait <= 0)
		{
			++_wakeup;
			_cv.notify_one();
		}
	}
};
#endif // !_CELL_SEMAPHORE_HPP
