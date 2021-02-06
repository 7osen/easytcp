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
				});//false时阻塞 防止虚假唤醒（线程被从等待状态唤醒了，但其实共享变量（即条件）并未变为true）
			--_wakeup;
		}
	}

	void Wakeup()
	{
		if (++_wait <= 0) //有进程在等待
		{
			++_wakeup;
			_cv.notify_one();
		}
	}
};
#endif // !_CELL_SEMAPHORE_HPP
