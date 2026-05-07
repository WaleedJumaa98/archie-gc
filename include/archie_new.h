#pragma once
#include "archie_object.h"
#include "vm.h"

// Constructors for each Archie object type.
// Each function allocates the object on the heap and registers it with the VM.
archie_object_t *new_archie_integer(vm_t *vm, int value);
archie_object_t *new_archie_float(vm_t *vm, float value);
archie_object_t *new_archie_string(vm_t *vm, char *value);
archie_object_t *new_archie_vector3(vm_t *vm, archie_object_t *x,
                                    archie_object_t *y, archie_object_t *z);
archie_object_t *new_archie_array(vm_t *vm, size_t size);
