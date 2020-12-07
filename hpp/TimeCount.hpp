#ifndef  _TimeCount_hpp_
#define _TimeCount_hpp_
#include <chrono>
using namespace std::chrono;
class TimeCount
{
public:
	TimeCount()
	{
		Update();
	}

	void Update()
	{
		_begin = high_resolution_clock::now();
	}

	double getSecond()
	{
		return getMircoSec() * 0.000001;
	}

	long long getMircoSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}
	time_t getMillSec()
	{
		return getMircoSec() * 0.001;
	}
	~TimeCount()
	{

	}

private:
	time_point<high_resolution_clock> _begin;
};

#endif // ! _TimeCount_hpp_
