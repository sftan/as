/**
 * AS - the open source Automotive Software on https://github.com/parai
 *
 * Copyright (C) 2017  AS <parai@foxmail.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#ifndef KERNEL_INTERNAL_H_
#define KERNEL_INTERNAL_H_
/* ============================ [ INCLUDES  ] ====================================================== */
#include "Os.h"
#include <sys/queue.h>
#include "portable.h"
#ifdef USE_SHELL
#include "shell.h"
#endif
/* ============================ [ MACROS    ] ====================================================== */
/*
 * BCC1 (only basic tasks, limited to one activation request per task and one task per
 * priority, while all tasks have different priorities)
 * BCC2 (like BCC1, plus more than one task per priority possible and multiple requesting
 * of task activation allowed)
 * ECC1 (like BCC1, plus extended tasks)
 * ECC2 (like ECC1, plus more than one task per priority possible and multiple requesting
 * of task activation allowed for basic tasks)
 */
enum {
	BCC1,
	BCC2,
	ECC1,
	ECC2
};

enum {
	STANDARD,
	EXTENDED
};
/*
 *  kernel call level
 */
#define TCL_NULL		((unsigned int) 0x00)	/* invalid kernel call level */
#define TCL_TASK		((unsigned int) 0x01)	/* task level */
#define TCL_ISR2		((unsigned int) 0x02)	/* interrupt type 2 ISR */
#define TCL_ERROR		((unsigned int) 0x04)	/* ErrorHook */
#define TCL_PREPOST		((unsigned int) 0x08)	/* PreTaskHook & PostTaskHook */
#define TCL_STARTUP		((unsigned int) 0x10)	/* StartupHook */
#define TCL_SHUTDOWN	((unsigned int) 0x20)	/* ShutdownHook */

#define INVALID_PRIORITY	((PriorityType)-1)

#ifdef OS_USE_ERROR_HOOK
/* OSErrorOne/OSErrorTwo/OSErrorThree
 * a. Each API MUST keep the parameter name the same with
 * the "param" of OSError_##api##_##param.
 * b. Each API block MUST have the "ercd" variable.
 */
/* for those API only with no parameter */
#define OSErrorNone(api) do { \
	if(ercd != E_OK) { \
		unsigned int savedLevel; \
		imask_t mask; \
		Irq_Save(mask); \
		_errorhook_svcid = OSServiceId_##api; \
		savedLevel = CallLevel; \
		CallLevel = TCL_ERROR; \
		ErrorHook(ercd); \
		CallLevel = savedLevel; \
		Irq_Restore(mask); \
	} \
} while(0)

/* for those API only with 1 parameter */
#define OSErrorOne(api, param) do { \
	if(ercd != E_OK) { \
		unsigned int savedLevel; \
		imask_t mask; \
		Irq_Save(mask); \
		_errorhook_svcid = OSServiceId_##api; \
		OSError_##api##_##param() = param; \
		savedLevel = CallLevel; \
		CallLevel = TCL_ERROR; \
		ErrorHook(ercd); \
		CallLevel = savedLevel; \
		Irq_Restore(mask); \
	} \
} while(0)

/* for those API with 2 parameters */
#define OSErrorTwo(api, param0, param1) do { \
	if(ercd != E_OK) { \
		unsigned int savedLevel; \
		imask_t mask; \
		Irq_Save(mask); \
		_errorhook_svcid = OSServiceId_##api; \
		OSError_##api##_##param0() = param0; \
		OSError_##api##_##param1() = param1; \
		savedLevel = CallLevel; \
		CallLevel = TCL_ERROR; \
		ErrorHook(ercd); \
		CallLevel = savedLevel; \
		Irq_Restore(mask); \
	} \
} while(0)

/* for those API with 3 parameters */
#define OSErrorThree(api, param0, param1, param2) do { \
	if(ercd != E_OK) { \
		unsigned int savedLevel; \
		imask_t mask; \
		Irq_Save(mask); \
		_errorhook_svcid = OSServiceId_##api; \
		OSError_##api##_##param0() = param0; \
		OSError_##api##_##param1() = param1; \
		OSError_##api##_##param2() = param2; \
		savedLevel = CallLevel; \
		CallLevel = TCL_ERROR; \
		ErrorHook(ercd); \
		CallLevel = savedLevel; \
		Irq_Restore(mask); \
	} \
} while(0)
#else
#define OSErrorNone(api)
#define OSErrorOne(api, param)
#define OSErrorTwo(api, param0, param1)
#define OSErrorThree(api, param0, param1, param2)
#endif

#ifdef OS_USE_STARTUP_HOOK
#define OSStartupHook() do { \
	unsigned int savedLevel; \
	imask_t mask; \
	Irq_Save(mask); \
	savedLevel = CallLevel; \
	CallLevel = TCL_STARTUP; \
	StartupHook(); \
	CallLevel = savedLevel; \
	Irq_Restore(mask); \
} while(0)
#else
#define OSStartupHook()
#endif

#ifdef OS_USE_SHUTDOWN_HOOK
#define OSShutdownHook(ercd) do { \
	unsigned int savedLevel; \
	imask_t mask; \
	Irq_Save(mask); \
	savedLevel = CallLevel; \
	CallLevel = TCL_SHUTDOWN; \
	ShutdownHook(ercd); \
	CallLevel = savedLevel; \
	Irq_Restore(mask); \
} while(0)
#else
#define OSShutdownHook(ercd)
#endif

#ifdef OS_USE_PRETASK_HOOK
#define OSPreTaskHook() do { \
	unsigned int savedLevel; \
	imask_t mask; \
	Irq_Save(mask); \
	savedLevel = CallLevel; \
	CallLevel = TCL_PREPOST; \
	PreTaskHook(); \
	CallLevel = savedLevel; \
	Irq_Restore(mask); \
} while(0)
#else
#define OSPreTaskHook()
#endif

#ifdef OS_USE_POSTTASK_HOOK
#define OSPostTaskHook() do { \
	unsigned int savedLevel; \
	imask_t mask; \
	Irq_Save(mask); \
	savedLevel = CallLevel; \
	CallLevel = TCL_PREPOST; \
	PostTaskHook(); \
	CallLevel = savedLevel; \
	Irq_Restore(mask); \
} while(0)
#else
#define OSPostTaskHook()
#endif

#define OS_IS_ALARM_STARTED(pVar) (NULL != ((pVar)->entry.tqe_prev))
#define OS_STOP_ALARM(pVar) do { ((pVar)->entry.tqe_prev) = NULL; } while(0)

#if(OS_PTHREAD_NUM > 0)
#define PTHREAD_DEFAULT_STACK_SIZE  4096
#define PTHREAD_DEFAULT_PRIORITY    (OS_PTHREAD_PRIORITY/2)
#endif
/* ============================ [ TYPES     ] ====================================================== */
typedef uint8					PriorityType;

typedef void	(*TaskMainEntryType)(void);
typedef void	(*AlarmMainEntryType)(void);

typedef struct
{
	PriorityType ceilPrio;
} ResourceConstType;

typedef struct
{
	PriorityType prevPrio;
	ResourceType prevRes;
} ResourceVarType;

typedef struct
{
	EventMaskType set;
	EventMaskType wait;
} EventVarType;

typedef struct
{
	void* pStack;
	uint32_t stackSize;
	TaskMainEntryType entry;
	#ifdef EXTENDED_TASK
	EventVarType* pEventVar;
	#endif
	#if (OS_STATUS == EXTENDED)
	boolean (*CheckAccess)(ResourceType);
	#endif
	PriorityType initPriority;
	PriorityType runPriority;
	const char* name;
	#ifdef MULTIPLY_TASK_ACTIVATION
	uint8 maxActivation;
	#endif
	boolean autoStart;
} TaskConstType;

typedef struct TaskVar
{
	TaskContextType context;
	PriorityType priority;
	const TaskConstType* pConst;
	#ifdef MULTIPLY_TASK_ACTIVATION
	uint8 activation;
	#endif
	StatusType state;
	ResourceType currentResource;
	#ifdef USE_SHELL
	uint32 actCnt;
	#endif
	#if(OS_PTHREAD_NUM > 0)
	/* generic entry for event/timer/mutex/semaphore etc. */
	TAILQ_ENTRY(TaskVar) entry;
	/* for sleep purpose */
	TickType sleep_tick;
	TAILQ_ENTRY(TaskVar) sentry;
	#endif
} TaskVarType;

struct AlarmVar;

typedef struct
{
	TickType value;
	TAILQ_HEAD(AlarmVarHead,AlarmVar) head;
} CounterVarType;

typedef struct
{
	const char* name;
	CounterVarType     *pVar;
	const AlarmBaseType base;
} CounterConstType;

typedef struct AlarmVar
{
	TickType value;
	TickType period;
	TAILQ_ENTRY(AlarmVar) entry;
} AlarmVarType;

typedef struct
{
	const char* name;
	AlarmVarType*     pVar;
	const CounterConstType* pCounter;
	void (*Start)(void);
	void (*Action)(void);
} AlarmConstType;

typedef struct
{
	const uint8 max;
	TaskType* pFIFO;
} ReadyFIFOType;
/* ============================ [ DECLARES  ] ====================================================== */
/* ============================ [ DATAS     ] ====================================================== */
extern TaskVarType* RunningVar;
extern TaskVarType* ReadyVar;
extern unsigned int CallLevel;
extern const TaskConstType TaskConstArray[TASK_NUM];
extern TaskVarType TaskVarArray[TASK_NUM+OS_PTHREAD_NUM];
extern CounterVarType CounterVarArray[COUNTER_NUM];
extern const CounterConstType CounterConstArray[COUNTER_NUM];
extern AlarmVarType AlarmVarArray[ALARM_NUM];
extern const AlarmConstType AlarmConstArray[ALARM_NUM];
/* ============================ [ LOCALS    ] ====================================================== */
/* ============================ [ FUNCTIONS ] ====================================================== */
extern void Os_TaskInit(void);
extern void Os_ResourceInit(void);
extern void Os_CounterInit(void);
extern void Os_AlarmInit(void);
extern void Os_StartAlarm(AlarmType AlarmID, TickType Start ,TickType Cycle);

extern void Os_PortInit(void);
extern void Os_PortInitContext(TaskVarType* pTaskVar);
extern void Os_PortStartDispatch(void);
extern void Os_PortDispatch(void);

extern void Sched_Init(void);
extern void Sched_AddReady(TaskType TaskID);
extern void Sched_GetReady(void);
extern void Sched_Preempt(void);
extern bool Sched_Schedule(void);
#if(OS_PTHREAD_NUM > 0)
extern void Sched_PosixAddReady(TaskType TaskID);
#endif
#endif /* KERNEL_INTERNAL_H_ */
