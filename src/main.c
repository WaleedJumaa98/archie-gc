#include "archie_new.h"
#include "archie_object.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

// Demo 1: Basic GC — object is freed when its frame is popped
static void demo_basic_gc() {
  printf("=== Demo 1: Basic GC ===\n");

  vm_t    *vm = vm_new();
  frame_t *f1 = vm_new_frame(vm);

  archie_object_t *s = new_archie_string(vm, "Hello from Archie!");
  frame_reference_object(f1, s);

  printf("Before GC (frame still active): objects in VM = %zu\n",
         vm->objects->count);

  vm_collect_garbage(vm);
  printf("After GC (frame still active): objects in VM = %zu\n",
         vm->objects->count);

  // Pop the frame — the string is now unreachable
  frame_free(vm_frame_pop(vm));
  vm_collect_garbage(vm);
  printf("After GC (frame popped): objects in VM = %zu (should be 0)\n\n",
         vm->objects->count);

  vm_free(vm);
}

// Demo 2: Multiple frames and scopes
static void demo_multiple_frames() {
  printf("=== Demo 2: Multiple Frames ===\n");

  vm_t    *vm = vm_new();
  frame_t *f1 = vm_new_frame(vm);
  frame_t *f2 = vm_new_frame(vm);

  archie_object_t *i1 = new_archie_integer(vm, 42);
  archie_object_t *i2 = new_archie_integer(vm, 100);
  // This float is never referenced by any frame — it's immediately garbage
  new_archie_float(vm, 3.14f);

  frame_reference_object(f1, i1);
  frame_reference_object(f2, i2);

  printf("Objects before GC: %zu\n", vm->objects->count);
  vm_collect_garbage(vm);
  printf("Objects after GC (unreferenced float collected): %zu\n",
         vm->objects->count);

  // Pop f2 — i2 becomes garbage
  frame_free(vm_frame_pop(vm));
  vm_collect_garbage(vm);
  printf("Objects after popping f2: %zu (should be 1)\n\n",
         vm->objects->count);

  frame_free(vm_frame_pop(vm));
  vm_free(vm);
}

// Demo 3: Cyclic reference — the classic refcounting failure case.
// Mark-and-sweep handles this correctly because reachability is checked
// from live frames, not from reference counts.
static void demo_cycle() {
  printf("=== Demo 3: Cyclic Reference ===\n");
  printf("(This is the case that breaks reference counting GCs)\n");

  vm_t    *vm = vm_new();
  frame_t *f1 = vm_new_frame(vm);

  // Create two arrays that point at each other
  archie_object_t *a = new_archie_array(vm, 1);
  archie_object_t *b = new_archie_array(vm, 1);

  archie_array_set(a, 0, b); // a -> b
  archie_array_set(b, 0, a); // b -> a (cycle!)

  // Reference only 'a' from the frame
  frame_reference_object(f1, a);

  printf("Objects before GC: %zu\n", vm->objects->count);
  vm_collect_garbage(vm);
  printf("Objects after GC (both reachable via frame): %zu\n",
         vm->objects->count);

  // Drop the frame — neither a nor b is reachable from any frame now
  frame_free(vm_frame_pop(vm));
  vm_collect_garbage(vm);
  printf("Objects after frame popped (cycle collected): %zu (should be 0)\n\n",
         vm->objects->count);

  vm_free(vm);
}

// Demo 4: Vector3 with nested objects
static void demo_vector3() {
  printf("=== Demo 4: Vector3 with Nested Objects ===\n");

  vm_t    *vm = vm_new();
  frame_t *f1 = vm_new_frame(vm);

  archie_object_t *x = new_archie_float(vm, 1.0f);
  archie_object_t *y = new_archie_float(vm, 2.0f);
  archie_object_t *z = new_archie_float(vm, 3.0f);
  archie_object_t *v = new_archie_vector3(vm, x, y, z);

  // Only reference the vector — the GC must trace into it to keep x, y, z alive
  frame_reference_object(f1, v);

  printf("Objects before GC: %zu\n", vm->objects->count);
  vm_collect_garbage(vm);
  printf("Objects after GC (vector + components kept alive): %zu\n",
         vm->objects->count);

  frame_free(vm_frame_pop(vm));
  vm_collect_garbage(vm);
  printf("Objects after frame popped (all freed): %zu (should be 0)\n\n",
         vm->objects->count);

  vm_free(vm);
}

int main(void) {
  printf("Archie Runtime — Garbage Collector Demo\n");
  printf("=========================================\n\n");

  demo_basic_gc();
  demo_multiple_frames();
  demo_cycle();
  demo_vector3();

  printf("All demos complete.\n");
  return 0;
}
