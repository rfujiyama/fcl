#include <stdlib.h>
#include <stdio.h>
#include "fcl_list.h"

struct my_node {
  int id;
  int priority;
  struct fcl_list_links links;
};

void print_node(struct my_node *n) {
  printf("prev: %p, n: %p, next: %p\n",
          (void*)n->links.prev, (void*)&n->links, (void*)n->links.next);
}

// generate the functions for my_node
FCL_LIST_DL_DEFINE(node, struct my_node, links)

int main() {
  // allocate and initialize the list head
  struct fcl_list_links *head = malloc(sizeof(*head));
  fcl_list_dl_init(head);

  // allocate and add 10 nodes to the list
  struct my_node *nodes = malloc(sizeof(*nodes) * 10);
  int i;
  for (i=0; i < 10; i++)
    node_list_insert_tail(head, &nodes[i]);

  // iterate over the list, printing and then removing the nodes
  struct fcl_list_links *iter, *tmp;
  struct my_node *entry;
  FCL_LIST_DL_EACH(head, iter, tmp) {
    entry = node_list_get_entry(iter);
    print_node(entry);
    node_list_remove(entry);
  }

  free(head);
  free(nodes);

  return 0;
}

