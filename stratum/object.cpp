#include <mutex>
#include "stratum.h"

mutex _mutex;

YAAMP_OBJECT *object_find(CommonList *list, int id, bool lock)
{
	if(lock) list->Enter();
	for(CLI li = list->first; li; li = li->next)
	{
		YAAMP_OBJECT *object = (YAAMP_OBJECT *)li->data;
		if(object->id == id)
		{
			if(lock)
			{
				object_lock(object);
				list->Leave();
			}

			return object;
		}
	}

	if(lock) list->Leave();
	return NULL;
}

void object_lock(YAAMP_OBJECT *object)
{
    unique_lock<mutex> lock(_mutex);
	if(object) object->lock_count++;
	lock.unlock();
}

void object_unlock(YAAMP_OBJECT *object)
{
    unique_lock<mutex> lock(_mutex);
	if(object) object->lock_count--;
	lock.unlock();
}

void object_delete(YAAMP_OBJECT *object)
{
    unique_lock<mutex> lock(_mutex);
	if(object) object->deleted = true;
	lock.unlock();
}

void object_prune(CommonList *list, YAAMP_OBJECT_DELETE_FUNC deletefunc)
{
	list->Enter();
	for(CLI li = list->first; li && list->count > 0; )
	{
		CLI todel = li;
		YAAMP_OBJECT *object = (YAAMP_OBJECT *)li->data;
		li = li->next;

        unique_lock<mutex> lock(_mutex);
		if(object) {
            if(object->deleted && !object->lock_count)
            {
                deletefunc(object);
                todel->data = NULL;
                list->Delete(todel);
            }

            else if(object->lock_count && object->unlock)
                object->lock_count--;
		}
        lock.unlock();
	}

	list->Leave();
}

void object_prune_debug(CommonList *list, YAAMP_OBJECT_DELETE_FUNC deletefunc)
{
	list->Enter();
	for(CLI li = list->first; li && list->count > 0; )
	{
		CLI todel = li;
		YAAMP_OBJECT *object = (YAAMP_OBJECT *)li->data;
		li = li->next;

        unique_lock<mutex> lock(_mutex);
		if(object) {
            if(object->deleted && object->lock_count)
                debuglog("object set for delete is locked\n");

            if(object->deleted && !object->lock_count)
            {
                deletefunc(object);
                todel->data = NULL;
                list->Delete(todel);
            }

            else if(object->lock_count && object->unlock)
                object->lock_count--;
		}
        lock.unlock();
	}

	if (list->count)
		debuglog("still %d objects in list\n", list->count);

	list->Leave();
}





