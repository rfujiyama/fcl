#include <stdlib.h>
#include <stdio.h>
#include "fcl_list.h"


struct my_node {
  int id;
  int priority;
  struct fcl_list_link links;
};

// declare the structs and functions for the list of my_node
FCL_LIST_LIFO_DECLARE(node, struct my_node, struct fcl_list_link, links)

// generate the functions for the list of my_node
FCL_LIST_LIFO_DEFINE(node, struct my_node, struct fcl_list_link, links)

void print_node(struct my_node *n) {
  printf("n: %p, next: %p\n",
          (void*)&n->links, (void*)n->links.next);
}

void print_head(struct node_list_head *head) {
  printf("head->first: %p\n", (void*)head->first);
}


int main() {
  // allocate and initialize the list head
  struct node_list_head *head = malloc(sizeof(*head));
  node_list_head_init(head);

  // allocate and add 10 nodes to the list
  struct my_node *nodes = malloc(sizeof(*nodes) * 10);
  int i;
  for (i=0; i < 10; i++) {
    //nodes[i].links.next = (void*)0x8;
    node_list_insert(head, &nodes[i]);
  }

  // iterate over the list, printing the nodes
  struct my_node *entry;
  struct fcl_list_link *iter, *tmp;
  print_head(head);
/*
  while (!node_list_is_empty(head)) {
    print_node(node_list_remove(head));
    print_head(head);
  }
*/
  FCL_LIST_LIFO_EACH(head, iter, tmp) {
    entry = node_list_get_entry(iter);
    print_node(entry);
  }

  free(head);
  free(nodes);

  return 0;
}

