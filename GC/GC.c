#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 256
#define MAX_OBJ 8

typedef enum
{
	OBJ_INT,
	OBJ_PAIR
}ObjectType;

typedef struct sObject
{
	ObjectType type;

	unsigned char marked;

	struct sObject* next;

	union 
	{
		int value;

		struct
		{
			struct sObject *head;
			struct sObject *tail;
		};
	};
}Object;

typedef struct
{
	Object* stack[STACK_SIZE];
	int stackSize;
	Object* firstoObject;
	int numObjects;
	int maxObjects;
}VM;

VM* newVM()
{
	VM *vm = malloc(sizeof(VM));
	vm->stackSize = 0;
	vm->firstoObject = NULL;
	vm->numObjects = 0;
	vm->maxObjects = MAX_OBJ;

	return vm;
}

void push(VM* vm, Object* value)
{
	vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm)
{
	return vm->stack[--vm->stackSize];
}

void mark(Object* object)
{
	if (object->marked)
		return;

	object->marked = 1;

	if (object->type == OBJ_PAIR)
	{
		mark(object->head);
		mark(object->tail);
	}
}

void markAll(VM* vm)
{
	for (int i = 0; i < vm->stackSize; i++)
	{
		mark(vm->stack[i]);
	}
}

void sweep(VM* vm)
{
	Object** object = &vm->firstoObject;

	while (*object)
	{
		if (!(*object)->marked)
		{
			Object* unreached = *object;
			*object = unreached->next;
			free(unreached);
		}
		else
		{
			(*object)->marked = 0;
			object = &(*object)->next;
		}
	}
}

void gc(VM* vm)
{
	int numObjects = vm->numObjects;

	markAll(vm);
	sweep(vm);

	vm->maxObjects = vm->numObjects == 0 ? MAX_OBJ : vm->numObjects * 2;

	printf("Collected %d objects, %d remaining.\n", numObjects - vm->numObjects,
		vm->numObjects);
}

Object* newObject(VM* vm, ObjectType type)
{
	if (vm->numObjects == vm->maxObjects)
		gc(vm);

	Object* object = (int*)malloc(sizeof(Object));
	object->type = type;
	object->marked = 0;

	object->next = vm->firstoObject;
	vm->firstoObject = object;

	return object;
}

void pushInt(VM* vm, int intValue)
{
	Object* object = newObject(vm, OBJ_INT);
	object->value = intValue;
	push(vm, object);
}

Object* pushPair(VM* vm)
{
	Object* object = newObject(vm, OBJ_PAIR);
	object->tail = pop(vm);
	object->head = pop(vm);

	push(vm, object);

	return object;
}

void freeVM(VM* vm) {
	vm->stackSize = 0;
	gc(vm);
	free(vm);
}

void test1() {
	printf("Test 1: Objects on stack are preserved.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);

	gc(vm);
	freeVM(vm);
}

void test2() {
	printf("Test 2: Unreached objects are collected.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pop(vm);
	pop(vm);

	gc(vm);
	freeVM(vm);
}

void test3() {
	printf("Test 3: Reach nested objects.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pushPair(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	pushPair(vm);
	pushPair(vm);

	gc(vm);
	freeVM(vm);
}

void test4() {
	printf("Test 4: Handle cycles.\n");
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	Object* a = pushPair(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	Object* b = pushPair(vm);

	a->tail = b;
	b->tail = a;

	gc(vm);
	freeVM(vm);
}

void perfTest() {
	printf("Performance Test.\n");
	VM* vm = newVM();

	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < 20; j++) {
			pushInt(vm, i);
		}

		for (int k = 0; k < 20; k++) {
			pop(vm);
		}
	}
	freeVM(vm);
}

int main(int argc, const char* argv[]) {
	test1();
	test2();
	test3();
	test4();
	perfTest();

	return 0;
}