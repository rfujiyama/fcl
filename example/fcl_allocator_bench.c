#include <stdio.h>            // printf
#include <sys/time.h>         // gettimeofday
#include "fcl_allocator.h"
#include "fcl_list.h"

struct my_node {
  int id;
  int priority;
  struct fcl_list_links links;
};

FCL_ALLOCATOR_LL_DEFINE(node, struct my_node, struct fcl_list_links, links,
                        LIFO)

FCL_LIST_DL_DEFINE(node, struct my_node, links)

// function declarations
double delta_seconds(struct timeval *s, struct timeval *e);

int main() {
  struct node_allocator node_alloc;
  int i, num_nodes;
  num_nodes = 100000;
  struct fcl_list_links head;
  struct timeval start, end;
  struct my_node *entry;
  struct fcl_list_links *iter, *tmp;


  gettimeofday(&start, NULL);
  node_allocator_init(&node_alloc, num_nodes, FCL_ALLOCATOR_OOM_POLICY_DOUBLE, 0);
  fcl_list_dl_init(&head);

  for (i=0; i < num_nodes; i++)
    node_list_insert_tail(&head, node_allocator_borrow(&node_alloc));

  FCL_LIST_DL_EACH(&head, iter, tmp) {
    entry = node_list_get_entry(iter);
    node_list_remove(entry);
    node_allocator_return(&node_alloc, entry);
  }

  node_allocator_freeall(&node_alloc);
  gettimeofday(&end, NULL);

  printf("node_allocator: %fs\n", delta_seconds(&start, &end));


  gettimeofday(&start, NULL);
  fcl_list_dl_init(&head);
  for (i=0; i < num_nodes; i++) {
    entry = malloc(sizeof(*entry));
    if (!entry)
      continue;
    node_list_insert_tail(&head, entry);
  }

  FCL_LIST_DL_EACH(&head, iter, tmp) {
    entry = node_list_get_entry(iter);
    node_list_remove(entry);
    free(entry);
  }
  gettimeofday(&end, NULL);

  printf("malloc/free: %fs\n", delta_seconds(&start, &end));

  return 0;
}

double delta_seconds(struct timeval *s, struct timeval *e) {
  double d;

  if (e->tv_sec < s->tv_sec) {
    return 0.0;
  } else {
    d = e->tv_sec - s->tv_sec;
  }

  if (e->tv_usec > s->tv_usec) {
    d+= (e->tv_usec - s->tv_usec) / 1000000.0;
  } else {
    d--;
    d+= (1000000 - s->tv_usec + e->tv_usec) / 1000000.0;
  }

  return d;
}

