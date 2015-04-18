#include <stdio.h>
#include "src/args.h"

int main(void) {
	char str[] = "prob=0.23&time=4 & repeat = 3& wait=true";
	arg_node_t *list = args_parse(str, '&');

	arg_node_t *head = list;
	while (head != NULL) {
		fprintf(stderr, "%s,%d", head->name, head->type);
		switch (head->type) {
			case ARG_VALUE_TYPE_INT:
				fprintf(stderr, ",%d", head->value.i);
				break;
			case ARG_VALUE_TYPE_FLOAT:
				fprintf(stderr, ",%f", head->value.f);
				break;
			case ARG_VALUE_TYPE_STR:
				fprintf(stderr, ",\"%s\"", head->value.s);
				break;
		}
		fputc('\n', stderr);
		head = head->next;
	}

	args_free(list);

	return 0;
}
