/*
 * check_tcpserver.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include <check.h>

#include <logger.h>
#include <eloop.h>
#include <tcpserver.h>

#include "test_utils.h"

#define HOST "localhost"
#define PORT 8000


static struct Logger *logger = NULL;
static struct ELoop *eloop = NULL;
static struct TcpServer *tcp_server = NULL;
static struct TcpConnection *tcp_connection = NULL;
static int gai_errno = 0;

static struct addrinfo *
allocaddrinfo(void)
{
  struct addrinfo *out = calloc(1, sizeof(struct addrinfo));
  assert(out);
  out->ai_addr = calloc(1, sizeof(struct sockaddr_in));
  assert(out->ai_addr);
  return out;
}

static void
filladdr4(struct addrinfo *out,
          const char      *host,
          const char      *port)
{
  struct sockaddr_in *addr4 = (struct sockaddr_in *)out->ai_addr;
  addr4->sin_family = AF_INET;
  inet_pton(out->ai_family, host, &(addr4->sin_addr));
  addr4->sin_port = htons(atoi(port));

  out->ai_addrlen = sizeof(struct sockaddr_in);
  out->ai_family = addr4->sin_family;
}

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res)
{
  /* not assert() because we want to deal with error cases */
  int ret = -1;
  if (node != NULL && service != NULL && res != NULL) {
    if (strcmp(node, "localhost") == 0) {
      struct addrinfo *out = allocaddrinfo();
      out->ai_socktype = SOCK_STREAM;
      filladdr4(out, "127.0.0.1", service);
      *res = out;
      ret = 0;
    } else if (strcmp(node, "notexistent") == 0) {
      gai_errno = EAI_FAIL;
    }
  }
  return ret;
}

void freeaddrinfo(struct addrinfo *res)
{
  if (res) {
    free(res->ai_addr);
    free(res);
  }
}

const char *gai_strerror(int errcode)
{
  const char *desc = NULL;
  switch (errcode) {
    case EAI_FAIL:
      desc = "The name server returned a permanent failure indication.";
      break;
    default:
      desc = "Unknown error.";
      break;
  }
  return desc;
}


static void
accept_func(struct TcpConnection *connection,
            const void           *data)
{
  struct ELoop *eloop = (struct ELoop *)data;

  tcp_connection = connection;

  event_loop_stop(eloop);
}

void
setup(void)
{
  logger = logger_new_null();
  eloop = event_loop_new(logger);
  tcp_server = tcp_server_new(logger, eloop);
  tcp_connection = NULL;
}

void teardown(void)
{
  if (tcp_connection != NULL)
    tcp_connection_destroy(tcp_connection);
  tcp_server_destroy(tcp_server);
  event_loop_destroy(eloop);
  logger_destroy(logger);
}

START_TEST(test_tcp_server_calls_callback_when_accepts_new_connections)
{
  int ret;
  int client_fd = -1;

  tcp_server_set_accept_callback(tcp_server, accept_func, eloop);
  ret = tcp_server_start_listen(tcp_server, HOST, PORT);
  ck_assert(ret == 0);

  ret = connect_to(HOST, PORT);
  ck_assert(ret >= 0);

  event_loop_run(eloop);

  ck_assert(tcp_connection != NULL);
}
END_TEST

START_TEST(test_tcp_server_dont_bind_on_not_existent_address)
{
  int ret = tcp_server_start_listen(tcp_server, "notexistent", PORT);

  ck_assert(ret != 0);
}
END_TEST

START_TEST(test_tcp_server_dont_bind_on_used_port)
{
  int other_server_fd = listen_to(HOST, PORT);
  int ret = tcp_server_start_listen(tcp_server, HOST, PORT);

  ck_assert(ret != 0);

  close(other_server_fd);
}
END_TEST

static Suite *
tcp_server_suite(void)
{
  Suite *s = suite_create("rapp.core.tcpserver");
  TCase *tc = tcase_create("rapp.core.tcpserver");

  tcase_add_checked_fixture (tc, setup, teardown);
  tcase_add_test(tc, test_tcp_server_calls_callback_when_accepts_new_connections);
  tcase_add_test(tc, test_tcp_server_dont_bind_on_not_existent_address);
  tcase_add_test(tc, test_tcp_server_dont_bind_on_used_port);

  suite_add_tcase(s, tc);

  return s;
}

int
main (void)
{
 int number_failed = 0;

 Suite *s = tcp_server_suite ();
 SRunner *sr = srunner_create (s);

 srunner_run_all (sr, CK_NORMAL);
 number_failed = srunner_ntests_failed (sr);
 srunner_free (sr);

 return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

