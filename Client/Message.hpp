#ifndef  _Message_hpp_
#define _Message_hpp_
#include <string>
using std::string;
struct DataHeader
{
	int DataLength;
	int HeaderLength;
	DataHeader(int length)
	{
		HeaderLength = sizeof(DataHeader) + length;
		DataLength = length;
	}
};
#endif // ! _Message_hpp_
