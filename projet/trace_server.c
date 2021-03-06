/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "trace.h"
#include <time.h>

reponse *
trace_1_svc(data *args, struct svc_req *rqstp)
{
	static reponse result;
	FILE *log = fopen("trace.log", "a");
	if (log < 0)
	{
		perror("Error file opening");
		result.errno = -1;
		return &result;
	}
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	fprintf(log, "%d-%02d-%02d %02d:%02d:%02d		%d		%d		\"%s\"\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, args->pid, args->type, args->message);
	printf("%d-%02d-%02d %02d:%02d:%02d		%d		%d		\"%s\"\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, args->pid, args->type, args->message);
	fclose(log);
	result.errno = 0;
	return &result;
}
