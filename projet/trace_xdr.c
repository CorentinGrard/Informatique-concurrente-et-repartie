/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "trace.h"

bool_t
xdr_data (XDR *xdrs, data *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->message, ~0))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->type))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->pid))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_reponse (XDR *xdrs, reponse *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->errno))
		 return FALSE;
	return TRUE;
}