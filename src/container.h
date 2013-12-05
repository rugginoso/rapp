#ifndef CONTAINER_H
#define CONTAINER_H

// Increment this on introdution of ABI incompatibilities.
#define ABI_VERSION 1

struct HTTPRequest;
struct HTTPResponseWriter;

struct Container {
  void (*serve)(struct Container *, struct HTTPRequest *, struct HTTPResponseWriter *);

  void (*destroy)(struct Container *);
};

int container_get_abi_version();

struct Container *container_new(void);

void container_destroy(struct Container *container);

void container_serve(struct Container *container, struct HTTPRequest *http_request, struct HTTPResponseWriter *response_writer);

#endif /* CONTAINER_H */
