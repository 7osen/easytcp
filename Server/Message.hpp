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
struct DataBody :DataHeader
{
	char data[10];
	DataBody(char* msg)
		:DataHeader(12)
	{
		strcpy(data, msg);
	}
};
#endif // ! _Message_hpp_
