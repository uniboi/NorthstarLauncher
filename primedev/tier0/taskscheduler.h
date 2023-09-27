#pragma once

#include <mutex>

class CTaskScheduler
{
  public:
	void AddTask(std::function<void()> fnTask);
	void RunFrame();
  private:
	std::mutex m_Mutex;
	std::vector<std::function<void()>> m_vTaskPool;
};

inline CTaskScheduler* g_pTaskScheduler = nullptr;
