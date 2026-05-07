#pragma once
#include "stack.h"
#include <stdbool.h>
#include <stddef.h>

// Forward declaration so ArchieObject can reference itself
typedef struct ArchieObject archie_object_t;

// Dynamic array of archie object pointers
typedef struct {
  size_t           size;
  archie_object_t **elements;
} archie_array_t;

// 3D vector of archie object pointers
typedef struct {
  archie_object_t *x;
  archie_object_t *y;
  archie_object_t *z;
} archie_vector_t;

// The type tag for each object
typedef enum ArchieObjectKind {
  INTEGER,
  FLOAT,
  STRING,
  VECTOR3,
  ARRAY,
} archie_object_kind_t;

// Tagged union holding the actual value
typedef union ArchieObjectData {
  int             v_int;
  float           v_float;
  char           *v_string;
  archie_vector_t v_vector3;
  archie_array_t  v_array;
} archie_object_data_t;

// The core object struct used by the Archie runtime
typedef struct ArchieObject {
  bool                 is_marked; // used by the mark-and-sweep GC
  archie_object_kind_t kind;
  archie_object_data_t data;
} archie_object_t;

void             archie_object_free(archie_object_t *obj);
bool             archie_array_set(archie_object_t *array, size_t index, archie_object_t *value);
archie_object_t *archie_array_get(archie_object_t *array, size_t index);
