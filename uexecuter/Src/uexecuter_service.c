#include "uexecuter_service.h"
#include <string.h>
const uexecuter_function_prototype_t *uexecuter_service_find_by_name(uexecuter_function_service_info_t *inst,
                                                                     const char *name_begin, const char *name_end)
{
    size_t name_len = name_end - name_begin;
    for (size_t i = 0; i < inst->n_function; ++i)
    {
        if (name_len != inst->function_prototypes[i].name_len)
            continue;
        if (memcmp(inst->function_prototypes[i].name, name_begin, name_len) == 0)
        {
            return &inst->function_prototypes[i];
        }
    }
    return NULL;
}

const uexecuter_function_prototype_t *uexecuter_service_find_by_address(uexecuter_function_service_info_t *inst,
                                                                        void *ptr)
{
    for (size_t i = 0; i < inst->n_function; ++i)
    {
        if (ptr == inst->function_prototypes[i].ptr)
            return &inst->function_prototypes[i];
    }
    return NULL;
}
