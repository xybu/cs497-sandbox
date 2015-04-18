#include <stdio.h>
#include <string.h>
#include "src/csv.h"

int main(void) {
	// the "123" is malformed and will be ignored.
	char test[] = "a, test, 			bbb,\"'hello' \\\"world!\",\nnext record, \"last record\"123";
	size_t len = strlen(test);
	size_t num_fields = 0;
	char **result = csv_parse(test, len, &num_fields);
	int i;

	fprintf(stderr, "%lu\n", num_fields);
	for (i = 0; i < num_fields; ++i) {
		csv_unescape(result[i]);
		fprintf(stderr, "%d: \"%s\"\n", i, result[i]);
	}

	return 0;
}
