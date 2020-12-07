#ifndef _Memory_hpp_
#define _Memory_hpp_
#include<stdlib.h>
#include<iostream>
#include<mutex>
#include<list>

class MemoryPool;

class MemoryBlock
{
public:
	MemoryBlock* _next;
	MemoryPool* _alloc;
	int _ref;
	bool _inPool;
	MemoryBlock()
	{

	}

	~MemoryBlock()
	{

	}

private:

};

class MemoryPool
{
protected:
	char* _pThis;
	MemoryBlock* _pHead;
	size_t _BlockSize;
	size_t _BlockNum;
	std::mutex _mutex;
public:
	MemoryPool()
	{
		_pHead = nullptr;
		_pThis = nullptr;
		_BlockSize = 0;
		_BlockNum = 0;
	}

	virtual ~MemoryPool()
	{
		if (_pThis)
			free(_pThis);
	}

	void* allocte(size_t size)
	{
		if (nullptr != _pHead)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			if (nullptr != _pHead)
			{
				MemoryBlock* pRet = _pHead;
				_pHead = _pHead->_next;
				pRet->_ref = 1;
				return ((char*)pRet + sizeof(MemoryBlock));
			}
		}
		MemoryBlock* pRet = (MemoryBlock*)malloc(size + sizeof(MemoryBlock));
		pRet->_ref = 1;
		pRet->_inPool = false;
		pRet->_next = nullptr;
		pRet->_alloc = this;
		return ((char*)pRet + sizeof(MemoryBlock));
	}

	void deallocate(void* p)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)p - sizeof(MemoryBlock));
		if (--(pBlock->_ref) != 0) return;
		if (pBlock->_inPool)
		{
			std::lock_guard<std::mutex> lock(_mutex);
			pBlock->_next = _pHead;
			_pHead = pBlock;
		}
		else
		{
			free(pBlock);
		}
	}

	void init()
	{
		_pThis = (char*)malloc(_BlockNum * (_BlockSize + sizeof(MemoryBlock)));
		_pHead = (MemoryBlock*)_pThis;

		MemoryBlock* p = _pHead;
		p->_ref = 0;
		p->_alloc = this;
		p->_inPool = true;
		p->_next = nullptr;

		MemoryBlock* p2;
		for (int i = 1; i < _BlockNum; i++)
		{
			p2 = (MemoryBlock*)((char*)p + (_BlockSize + sizeof(MemoryBlock)));
			p2->_ref = 0;
			p2->_alloc = this;
			p2->_inPool = true;
			p2->_next = nullptr;
			p->_next = p2;
			p = p2;
		}
	}
};

template <size_t size, size_t num>
class MemoryPoolt :public MemoryPool
{
public:
	MemoryPoolt()
	{
		_BlockSize = size;
		_BlockNum = num;
	}
	~MemoryPoolt()
	{

	}

private:

};


class MemoryManager
{
private:
	MemoryManager()
	{
		SetMap(0, 4, &memp4);
		SetMap(5, 8, &memp8);
		SetMap(9, 16, &memp16);
		SetMap(17, 32, &memp32);
		SetMap(33, 1024, &memp1024);
		SetMap(1025, 10240, &memp10240);
	}

	void SetMap(int begin, int end, MemoryPool* pool)
	{
		pool->init();
		for (int i = begin; i <= end; i++)
			_map[i] = pool;
	}

	~MemoryManager()
	{

	}
	static const size_t MaxmemSize = 10240;
	MemoryPoolt<4, 5000> memp4;
	MemoryPoolt<8, 5000> memp8;
	MemoryPoolt<16, 2500> memp16;
	MemoryPoolt<32, 1200> memp32;
	MemoryPoolt<1024, 1000> memp1024;
	MemoryPoolt<10240, 1000> memp10240;

	MemoryPool* _map[MaxmemSize + 1];


public:

	static MemoryManager& getInstance()
	{
		static MemoryManager _instance;
		return _instance;
	}

	void* allocate(size_t size)
	{

		if (size <= MaxmemSize)
		{
		
			return _map[size]->allocte(size);
		}
		else
		{
			MemoryBlock* pRet = (MemoryBlock*)malloc(size + sizeof(MemoryBlock));
			pRet->_ref = 1;
			pRet->_inPool = false;
			pRet->_next = nullptr;
			pRet->_alloc = nullptr;
			return (char*)pRet + sizeof(MemoryBlock);
		}
	}

	void deallocate(void* p)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)p - sizeof(MemoryBlock));
		if (pBlock->_ref != 1)
		{
			pBlock->_ref--;
			return;
		}
		if (pBlock->_inPool)
		{
			pBlock->_alloc->deallocate(p);
		}
		else
		{
			free(pBlock);
		}
	}
};



void* operator new(size_t size)
{
	return MemoryManager::getInstance().allocate(size);
}

void* operator new[](size_t size)
{
	return MemoryManager::getInstance().allocate(size);
}

void operator delete(void* p)
{
	MemoryManager::getInstance().deallocate(p);
}

void operator delete[](void* p)
{
	MemoryManager::getInstance().deallocate(p);
}


void* mem_alloc(size_t size)
{
	return MemoryManager::getInstance().allocate(size);
}

void mem_free(void* p)
{
	MemoryManager::getInstance().deallocate(p);
}


template <typename Type>
class ObjectPool
{
public:
	ObjectPool(int size)
		:_size(size)
	{
		_list.clear();
		for (int i = 0; i < size; i++)
			_list.push_front(new Type());
	}
	~ObjectPool()
	{
		for (; !_list.empty();)
		{
			delete _list.front();
			_list.pop_front();
		}
	}
	Type* Get_Object()
	{
		_m.lock();
		if (!_size) return (new Type());
		Type* p = _list.front();
		_list.pop_front();
		_size--;
		_m.unlock();
		return p;
	}
	void RetrunObject(Type* p)
	{
		_m.lock();
		_list.push_front(p);
		_size++;
		_m.unlock();
	}

private:
	int _size;
	std::list<Type*> _list;
	std::mutex _m;
};

#endif // !_Memory_hpp_