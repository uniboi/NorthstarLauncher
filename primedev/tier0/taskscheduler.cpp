#include "tier0/taskscheduler.h"

//-----------------------------------------------------------------------------
// Purpose: Adds a task to the pool
//-----------------------------------------------------------------------------
void CTaskScheduler::AddTask(std::function<void()> fnTask)
{
	std::lock_guard<std::mutex> Guard(m_Mutex);

	m_vTaskPool.push_back(fnTask);
}

//-----------------------------------------------------------------------------
// Purpose: Runs all tasks for this frame
//-----------------------------------------------------------------------------
void CTaskScheduler::RunFrame()
{
	std::lock_guard<std::mutex> Guard(m_Mutex);

	for (std::function<void()> fnTask : m_vTaskPool)
	{
		fnTask();
	}

	m_vTaskPool.clear();
}
