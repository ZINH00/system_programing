#pragma once
class CGlobalRingBuffer
{
	CRingBuffer m_Buffer;
	CGlobalRingBuffer(void);
	~CGlobalRingBuffer(void);
public:
	void* Alloc(size_t tSize);
	static CGlobalRingBuffer* GetInstance(void)
	{
		static CGlobalRingBuffer instance;
		return &instance;
	}
};
inline CGlobalRingBuffer* RingBuffer(void)
{
	return CGlobalRingBuffer::GetInstance();
}
