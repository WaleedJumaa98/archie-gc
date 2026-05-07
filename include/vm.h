#pragma once
#include "archie_object.h"
#include "stack.h"

// The virtual machine tracks all live objects and active stack frames.
// It is the root of the mark-and-sweep garbage collector.
typedef struct VirtualMachine {
  stack_t *frames;  // stack of frame_t pointers (active scopes)
  stack_t *objects; // stack of archie_object_t pointers (every live object)
} vm_t;

// A stack frame represents one scope (e.g. a function call).
// It holds references to the objects used in that scope.
typedef struct StackFrame {
  stack_t *references; // archie_object_t pointers referenced in this scope
} frame_t;

// --- Garbage collection ---
void mark(vm_t *vm);   // Mark all objects reachable from active frames
void trace(vm_t *vm);  // Trace through containers to mark nested objects
void sweep(vm_t *vm);  // Free all unmarked objects, reset marks on survivors
void vm_collect_garbage(vm_t *vm); // Run a full GC cycle: mark -> trace -> sweep

// --- Trace helpers ---
void trace_blacken_object(stack_t *gray_objects, archie_object_t *obj);
void trace_mark_object(stack_t *gray_objects, archie_object_t *obj);

// --- VM lifecycle ---
vm_t   *vm_new();
void    vm_free(vm_t *vm);
void    vm_track_object(vm_t *vm, archie_object_t *obj);

// --- Frame management ---
frame_t *vm_new_frame(vm_t *vm);
void     vm_frame_push(vm_t *vm, frame_t *frame);
frame_t *vm_frame_pop(vm_t *vm);
void     frame_free(frame_t *frame);
void     frame_reference_object(frame_t *frame, archie_object_t *obj);
