#define _USE_MATH_DEFINES
#include "mcv_platform.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include "navmesh_builder.h"

#ifdef WIN32
//#	define snprintf _snprintf
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

rcBuildContext::rcBuildContext() :
	m_messageCount(0),
	m_textPoolSize(0)
{
	resetTimers();
}

rcBuildContext::~rcBuildContext()
{
}

// Virtual functions for custom implementations.
void rcBuildContext::doResetLog()
{
	m_messageCount = 0;
	m_textPoolSize = 0;
}

void rcBuildContext::doLog(const rcLogCategory category, const char* msg, const int len)
{
	if (!len) return;
	if (m_messageCount >= MAX_MESSAGES)
		return;
	char* dst = &m_textPool[m_textPoolSize];
	int n = TEXT_POOL_SIZE - m_textPoolSize;
	if (n < 2)
		return;
	char* cat = dst;
	char* text = dst+1;
	const int maxtext = n-1;
	// Store category
	*cat = (char)category;
	// Store message
	const int count = rcMin(len+1, maxtext);
	memcpy(text, msg, count);
	text[count-1] = '\0';
	m_textPoolSize += 1 + count;
	m_messages[m_messageCount++] = dst;
}

void rcBuildContext::doResetTimers()
{
	for (int i = 0; i < RC_MAX_TIMERS; ++i)
		m_accTime[i] = -1;
}

void rcBuildContext::doStartTimer(const rcTimerLabel label)
{
	m_startTime[label] = getPerfTime();
}

void rcBuildContext::doStopTimer(const rcTimerLabel label)
{
	const TimeVal endTime = getPerfTime();
	const int deltaTime = (int)(endTime - m_startTime[label]);
	if (m_accTime[label] == -1)
		m_accTime[label] = deltaTime;
	else
		m_accTime[label] += deltaTime;
}

int rcBuildContext::doGetAccumulatedTime(const rcTimerLabel label) const
{
	return m_accTime[label];
}

void rcBuildContext::dumpLog(const char* format, ...)
{
	// Print header.
	va_list ap;
	va_start(ap, format);
	printf(format, ap);
	va_end(ap);
  printf( "\n" );
	
	// Print messages
	const int TAB_STOPS[4] = { 28, 36, 44, 52 };
	for (int i = 0; i < m_messageCount; ++i)
	{
		const char* msg = m_messages[i]+1;
		int n = 0;
		while (*msg)
		{
			if (*msg == '\t')
			{
				int count = 1;
				for (int j = 0; j < 4; ++j)
				{
					if (n < TAB_STOPS[j])
					{
						count = TAB_STOPS[j] - n;
						break;
					}
				}
				while (--count)
				{
          printf( " " );
					n++;
				}
			}
			else
			{
        printf( msg );
				n++;
			}
			msg++;
		}
    printf( "\n" );
	}
}

int rcBuildContext::getLogCount() const
{
	return m_messageCount;
}

const char* rcBuildContext::getLogText(const int i) const
{
	return m_messages[i]+1;
}
