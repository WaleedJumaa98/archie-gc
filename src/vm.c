#include "vm.h"
#include "archie_object.h"
#include "stack.h"
#include <stdlib.h>

// --- VM lifecycle ---

vm_t *vm_new() {
  vm_t *vm = malloc(sizeof(vm_t));
  if (vm == NULL) {
    return NULL;
  }
  vm->frames  = stack_new(8);
  vm->objects = stack_new(8);
  return vm;
}

void vm_free(vm_t *vm) {
  // Free each frame and then the frames stack
  for (size_t i = 0; i < vm->frames->count; i++) {
    frame_free(vm->frames->data[i]);
  }
  stack_free(vm->frames);

  // Free each tracked object and then the objects stack
  for (size_t i = 0; i < vm->objects->count; i++) {
    archie_object_free(vm->objects->data[i]);
  }
  stack_free(vm->objects);

  free(vm);
}

// Register a newly created object with the VM so the GC can track it.
void vm_track_object(vm_t *vm, archie_object_t *obj) {
  stack_push(vm->objects, obj);
}

// --- Frame management ---

void vm_frame_push(vm_t *vm, frame_t *frame) {
  stack_push(vm->frames, frame);
}

frame_t *vm_frame_pop(vm_t *vm) {
  return stack_pop(vm->frames);
}

frame_t *vm_new_frame(vm_t *vm) {
  frame_t *frame = malloc(sizeof(frame_t));
  if (frame == NULL) {
    return NULL;
  }
  frame->references = stack_new(8);
  vm_frame_push(vm, frame);
  return frame;
}

void frame_free(frame_t *frame) {
  stack_free(frame->references);
  free(frame);
}

// Mark an object as referenced by a frame so the GC won't collect it.
void frame_reference_object(frame_t *frame, archie_object_t *obj) {
  stack_push(frame->references, obj);
}

// --- Garbage collection ---

// Phase 1: Mark all objects directly referenced by active stack frames.
void mark(vm_t *vm) {
  for (size_t i = 0; i < vm->frames->count; i++) {
    frame_t *frame = vm->frames->data[i];
    for (size_t j = 0; j < frame->references->count; j++) {
      archie_object_t *obj = frame->references->data[j];
      obj->is_marked = true;
    }
  }
}

// Phase 2: Trace through container objects to mark nested/reachable objects.
void trace(vm_t *vm) {
  stack_t *gray_objects = stack_new(8);
  if (gray_objects == NULL) {
    return;
  }

  // Seed the gray set with all objects already marked by the mark phase
  for (size_t i = 0; i < vm->objects->count; i++) {
    archie_object_t *obj = vm->objects->data[i];
    if (obj->is_marked) {
      stack_push(gray_objects, obj);
    }
  }

  // Process each gray object: mark its children and move it to black
  while (gray_objects->count > 0) {
    trace_blacken_object(gray_objects, stack_pop(gray_objects));
  }

  stack_free(gray_objects);
}

// Mark an object gray (reachable but children not yet traced).
void trace_mark_object(stack_t *gray_objects, archie_object_t *obj) {
  if (obj == NULL || obj->is_marked) {
    return;
  }
  obj->is_marked = true;
  stack_push(gray_objects, obj);
}

// Blacken an object: trace its children and mark them gray.
void trace_blacken_object(stack_t *gray_objects, archie_object_t *obj) {
  switch (obj->kind) {
  case INTEGER:
  case FLOAT:
  case STRING:
    // Primitive types have no child objects to trace
    break;

  case VECTOR3: {
    archie_vector_t vec = obj->data.v_vector3;
    trace_mark_object(gray_objects, vec.x);
    trace_mark_object(gray_objects, vec.y);
    trace_mark_object(gray_objects, vec.z);
    break;
  }

  case ARRAY: {
    for (size_t i = 0; i < obj->data.v_array.size; i++) {
      trace_mark_object(gray_objects, obj->data.v_array.elements[i]);
    }
    break;
  }
  }
}

// Phase 3: Free all unmarked objects. Reset marks on survivors for next cycle.
void sweep(vm_t *vm) {
  for (size_t i = 0; i < vm->objects->count; i++) {
    archie_object_t *obj = vm->objects->data[i];
    if (obj->is_marked) {
      // Object is still reachable: clear mark for the next GC cycle
      obj->is_marked = false;
    } else {
      // Object is unreachable: free it and null out the slot
      archie_object_free(obj);
      vm->objects->data[i] = NULL;
    }
  }
  // Compact the objects stack by removing NULL slots
  stack_remove_nulls(vm->objects);
}

// Run a full garbage collection cycle.
void vm_collect_garbage(vm_t *vm) {
  mark(vm);
  trace(vm);
  sweep(vm);
}
