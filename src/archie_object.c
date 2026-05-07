#include "archie_object.h"
#include "archie_new.h"
#include <stdlib.h>
#include <string.h>

// Free an object and any memory it uniquely owns.
// Contained objects (e.g. vector components, array elements) are NOT freed
// here because the VM tracks and frees them independently during the sweep phase.
void archie_object_free(archie_object_t *obj) {
  switch (obj->kind) {
  case INTEGER:
  case FLOAT:
    // Data is stored inline in the struct, nothing extra to free.
    break;

  case STRING:
    // The char* was heap-allocated separately, so free it first.
    free(obj->data.v_string);
    break;

  case VECTOR3:
    // x, y, z are tracked by the VM; the GC will free them separately.
    break;

  case ARRAY:
    // Free the pointer array itself, but not the objects it points to.
    // The VM will free those when their turn comes in the sweep.
    free(obj->data.v_array.elements);
    break;
  }

  free(obj);
}

// Set an element in an archie array at the given index.
bool archie_array_set(archie_object_t *array, size_t index,
                      archie_object_t *value) {
  if (array == NULL || value == NULL) {
    return false;
  }
  if (array->kind != ARRAY) {
    return false;
  }
  if (index >= array->data.v_array.size) {
    return false;
  }
  array->data.v_array.elements[index] = value;
  return true;
}

// Get an element from an archie array at the given index.
archie_object_t *archie_array_get(archie_object_t *array, size_t index) {
  if (array == NULL) {
    return NULL;
  }
  if (array->kind != ARRAY) {
    return NULL;
  }
  if (index >= array->data.v_array.size) {
    return NULL;
  }
  return array->data.v_array.elements[index];
}

// Add two archie objects together.
// Supports INTEGER, FLOAT, STRING, VECTOR3, and ARRAY.
// Returns NULL for unsupported type combinations.
archie_object_t *archie_add(vm_t *vm, archie_object_t *a, archie_object_t *b) {
  if (a == NULL || b == NULL) {
    return NULL;
  }

  switch (a->kind) {

  case INTEGER:
    switch (b->kind) {
    case INTEGER:
      return new_archie_integer(vm, a->data.v_int + b->data.v_int);
    case FLOAT:
      return new_archie_float(vm, (float)a->data.v_int + b->data.v_float);
    default:
      return NULL;
    }

  case FLOAT:
    switch (b->kind) {
    case FLOAT:
      return new_archie_float(vm, a->data.v_float + b->data.v_float);
    default:
      // Try the reversed combination (e.g. FLOAT + INTEGER)
      return archie_add(vm, b, a);
    }

  case STRING:
    switch (b->kind) {
    case STRING: {
      int   a_len = strlen(a->data.v_string);
      int   b_len = strlen(b->data.v_string);
      int   len   = a_len + b_len + 1;
      char *dst   = malloc(len * sizeof(char));
      dst[0]      = '\0';
      strcat(dst, a->data.v_string);
      strcat(dst, b->data.v_string);
      archie_object_t *obj = new_archie_string(vm, dst);
      free(dst);
      return obj;
    }
    default:
      return NULL;
    }

  case VECTOR3:
    switch (b->kind) {
    case VECTOR3:
      return new_archie_vector3(
          vm,
          archie_add(vm, a->data.v_vector3.x, b->data.v_vector3.x),
          archie_add(vm, a->data.v_vector3.y, b->data.v_vector3.y),
          archie_add(vm, a->data.v_vector3.z, b->data.v_vector3.z));
    default:
      return NULL;
    }

  case ARRAY:
    switch (b->kind) {
    case ARRAY: {
      size_t a_len  = a->data.v_array.size;
      size_t b_len  = b->data.v_array.size;
      archie_object_t *result = new_archie_array(vm, a_len + b_len);
      for (size_t i = 0; i < a_len; i++) {
        archie_array_set(result, i, archie_array_get(a, i));
      }
      for (size_t i = 0; i < b_len; i++) {
        archie_array_set(result, i + a_len, archie_array_get(b, i));
      }
      return result;
    }
    default:
      return NULL;
    }

  default:
    return NULL;
  }
}
