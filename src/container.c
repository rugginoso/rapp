#include <stdio.h>
#include <assert.h>

#include "container.h"


void container_destroy(struct Container *container)
{
  assert(container != NULL);

  if (container->destroy != NULL)
    container->destroy(container);
}

void
container_serve(struct Container          *container,
                struct HTTPRequest        *http_request,
                struct HTTPResponseWriter *response_writer)
{
  assert(container != NULL);
  assert(http_request != NULL);
  assert(response_writer != NULL);

  if (container->serve != NULL)
    container->serve(container, http_request, response_writer);
}
