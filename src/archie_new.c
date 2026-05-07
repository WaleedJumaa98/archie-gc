#include "archie_new.h"
#include "archie_object.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>

// Internal helper: allocate a zeroed archie object and register it with the VM.
static archie_object_t *_new_archie_object(vm_t *vm) {
  archie_object_t *obj = calloc(1, sizeof(archie_object_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->is_marked = false;
  vm_track_object(vm, obj);
  return obj;
}

archie_object_t *new_archie_integer(vm_t *vm, int value) {
  archie_object_t *obj = _new_archie_object(vm);
  if (obj == NULL) {
    return NULL;
  }
  obj->kind        = INTEGER;
  obj->data.v_int  = value;
  return obj;
}

archie_object_t *new_archie_float(vm_t *vm, float value) {
  archie_object_t *obj = _new_archie_object(vm);
  if (obj == NULL) {
    return NULL;
  }
  obj->kind          = FLOAT;
  obj->data.v_float  = value;
  return obj;
}

archie_object_t *new_archie_string(vm_t *vm, char *value) {
  archie_object_t *obj = _new_archie_object(vm);
  if (obj == NULL) {
    return NULL;
  }

  int   len = strlen(value);
  char *dst = malloc(len + 1);
  if (dst == NULL) {
    free(obj);
    return NULL;
  }

  strcpy(dst, value);
  obj->kind           = STRING;
  obj->data.v_string  = dst;
  return obj;
}

archie_object_t *new_archie_vector3(vm_t *vm, archie_object_t *x,
                                    archie_object_t *y, archie_object_t *z) {
  if (x == NULL || y == NULL || z == NULL) {
    return NULL;
  }

  archie_object_t *obj = _new_archie_object(vm);
  if (obj == NULL) {
    return NULL;
  }

  obj->kind          = VECTOR3;
  obj->data.v_vector3 = (archie_vector_t){.x = x, .y = y, .z = z};
  return obj;
}

archie_object_t *new_archie_array(vm_t *vm, size_t size) {
  archie_object_t *obj = _new_archie_object(vm);
  if (obj == NULL) {
    return NULL;
  }

  // calloc ensures all element pointers start as NULL
  archie_object_t **elements = calloc(size, sizeof(archie_object_t *));
  if (elements == NULL) {
    free(obj);
    return NULL;
  }

  obj->kind       = ARRAY;
  obj->data.v_array = (archie_array_t){.size = size, .elements = elements};
  return obj;
}
