/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "trace_client.h"

void send_rpc_msg(char msg[], int type, pid_t pid)
{
	CLIENT *clnt;
	reponse *result;
	data arguments;
	arguments.message = msg;
	arguments.type = type;
	arguments.pid = pid;

#ifndef DEBUG
	clnt = clnt_create("127.0.0.1", CALCUL, VERSION_UN, "udp");
	if (clnt == NULL)
	{
		clnt_pcreateerror("127.0.0.1");
		exit(1);
	}
#endif /* DEBUG */

	result = trace_1(&arguments, clnt);
	if (result == (reponse *)NULL)
	{
		clnt_perror(clnt, "call failed");
	}
#ifndef DEBUG
	clnt_destroy(clnt);
#endif /* DEBUG */
}