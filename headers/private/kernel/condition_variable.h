/*
 * Copyright 2007-2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2019, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_CONDITION_VARIABLE_H
#define _KERNEL_CONDITION_VARIABLE_H


#include <OS.h>

#include <debug.h>

#ifdef __cplusplus

#include <util/DoublyLinkedList.h>
#include <util/OpenHashTable.h>


struct mutex;
struct recursive_lock;
struct ConditionVariable;


struct ConditionVariableEntry
	: DoublyLinkedListLinkImpl<ConditionVariableEntry> {
public:
								ConditionVariableEntry();
								~ConditionVariableEntry();

			bool				Add(const void* object);
			status_t			Wait(uint32 flags = 0, bigtime_t timeout = 0);
			status_t			Wait(const void* object, uint32 flags = 0,
									bigtime_t timeout = 0);

			ConditionVariable*	Variable() const;

private:
	inline	void				_AddToLockedVariable(ConditionVariable* variable);
			void				_RemoveFromVariable();

private:
			ConditionVariable*	fVariable;
			Thread*				fThread;
			status_t			fWaitStatus;

			friend struct ConditionVariable;
};


struct ConditionVariable {
public:
			void				Init(const void* object,
									const char* objectType);
									// for anonymous (unpublished) cvars

			void				Publish(const void* object,
									const char* objectType);
			void				Unpublish();

	inline	int32				NotifyOne(status_t result = B_OK);
	inline	int32				NotifyAll(status_t result = B_OK);

	static	int32				NotifyOne(const void* object, status_t result);
	static	int32				NotifyAll(const void* object, status_t result);

			void				Add(ConditionVariableEntry* entry);
			int32				EntriesCount()		{ return atomic_get(&fEntriesCount); }

	// Convenience methods, no ConditionVariableEntry required.
			status_t			Wait(uint32 flags = 0, bigtime_t timeout = 0);
			status_t			Wait(mutex* lock, uint32 flags = 0, bigtime_t timeout = 0);
			status_t			Wait(recursive_lock* lock, uint32 flags = 0, bigtime_t timeout = 0);

			const void*			Object() const		{ return fObject; }
			const char*			ObjectType() const	{ return fObjectType; }

	static	void				ListAll();
			void				Dump() const;

private:
	static 	int32				_Notify(const void* object, bool all, status_t result);
			int32				_Notify(bool all, status_t result);
			int32				_NotifyLocked(bool all, status_t result);

protected:
			typedef DoublyLinkedList<ConditionVariableEntry> EntryList;

			const void*			fObject;
			const char*			fObjectType;

			spinlock			fLock;
			EntryList			fEntries;
			int32				fEntriesCount;

			ConditionVariable*	fNext;

			friend struct ConditionVariableEntry;
			friend struct ConditionVariableHashDefinition;
			friend ssize_t debug_condition_variable_type_strlcpy(ConditionVariable* cvar,
				char* name, size_t size);
};


inline int32
ConditionVariable::NotifyOne(status_t result)
{
	return _Notify(false, result);
}


inline int32
ConditionVariable::NotifyAll(status_t result)
{
	return _Notify(true, result);
}


extern "C" {
#endif	// __cplusplus

extern void condition_variable_init();

#ifdef __cplusplus
}	// extern "C"
#endif

#endif	/* _KERNEL_CONDITION_VARIABLE_H */
