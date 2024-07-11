//
// Created by hooman on 7/8/24.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <oci.h>

//[[protocol:]//]host1{,host12}[:port1]{,host2:port2}[/service_name][:server][/instance_name][?parameter_name=value{&parameter_name=value}]

const char * username = "mansi";
const char * password = "mansi123";
const char * database = "TCP://172.16.60.250:1521/ORCLCDB";

OCIEnv * oci_env = NULL;
OCIError * oci_err = NULL;
OCIServer * oci_server = NULL;
OCISvcCtx * oci_service = NULL;
OCISession * oci_auth = NULL;

void checkerr(OCIError * errhp, sword status)
{
	text errbuf[512];
	sb4 errcode = 0;

	switch (status)
	{
		case OCI_SUCCESS:
			break;
		case OCI_SUCCESS_WITH_INFO:
			(void) printf("Error - OCI_SUCCESS_WITH_INFO\n");
			break;
		case OCI_NEED_DATA:
			(void) printf("Error - OCI_NEED_DATA\n");
			break;
		case OCI_NO_DATA:
			(void) printf("Error - OCI_NODATA\n");
			break;
		case OCI_ERROR:
			(void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
			(void) printf("Error - %.*s\n", 512, errbuf);
			break;
		case OCI_INVALID_HANDLE:
			(void) printf("Error - OCI_INVALID_HANDLE\n");
			break;
		case OCI_STILL_EXECUTING:
			(void) printf("Error - OCI_STILL_EXECUTE\n");
			break;
		case OCI_CONTINUE:
			(void) printf("Error - OCI_CONTINUE\n");
			break;
		default:
			break;
	}
}

void exec_stmt()
{
	const char select_log_sql[] = "select * from tms.logs order by id asc";
	sword errcode = 0;
	OCIStmt * oci_stmt = NULL;

	errcode = OCIStmtPrepare2(oci_service, &oci_stmt, oci_err, select_log_sql, (ub4) (sizeof (select_log_sql) - 1),
							  NULL, 0, (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
	if (errcode != 0)
	{
		printf("OCIStmtPrepare2 failed with errcode = %d.\n", errcode);
		checkerr(oci_err, errcode);
		return;
	}

	errcode = OCIStmtExecute(oci_service, oci_stmt, oci_err, (ub4) 0, (ub4) 0, NULL, NULL, OCI_DEFAULT);
	if (errcode < 0)
	{
		printf("OCIStmtExecute failed with errcode = %d.\n", errcode);
		checkerr(oci_err, errcode);
	}

	ub2 dtype;
	ub4 counter = 1;
	OCIParam * mypard = NULL;
	errcode = OCIParamGet(oci_stmt, OCI_HTYPE_STMT, oci_err, &mypard, counter);
	while (errcode == OCI_SUCCESS)
	{
		errcode = OCIAttrGet(mypard, (ub4) OCI_DTYPE_PARAM, &dtype, (ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE, oci_err);
		if (errcode != 0)
		{
			printf("OCIAttrGet type %d failed with errcode = %d.\n", OCI_DTYPE_PARAM, errcode);
			checkerr(oci_err, errcode);
		}

		errcode = OCIParamGet(oci_stmt, OCI_HTYPE_STMT, oci_err, &mypard, ++counter);
	}

	errcode = OCIHandleFree(oci_stmt, (ub4) OCI_HTYPE_STMT);
	if (errcode != 0)
	{
		printf("OCIHandleFree type %d failed with errcode = %d.\n", OCI_HTYPE_STMT, errcode);
		checkerr(oci_err, errcode);
	}
}

int main(int argc, char * argv[])
{
	int main_ret_code = -1;
	sword errcode = 0;

	errcode = OCIEnvCreate((OCIEnv **) &oci_env, (ub4) OCI_DEFAULT,
						   0, (dvoid * (*)(dvoid *,size_t)) 0,
						   (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
						   (void (*)(dvoid *, dvoid *)) 0, (size_t) 0, NULL);
	if (errcode != 0)
	{
		printf("OCIEnvCreate failed with errcode = %d.\n", errcode);
		checkerr(oci_err, errcode);
		goto _exit_terminate;
	}

	errcode = OCIHandleAlloc(oci_env, &oci_err, OCI_HTYPE_ERROR, (size_t) 0, NULL);
	if (errcode != 0)
	{
		printf("OCIHandleAlloc %d failed with errcode = %d.\n", OCI_HTYPE_ERROR, errcode);
		checkerr(oci_err, errcode);
		goto _exit_env;
	}

	errcode = OCIHandleAlloc(oci_env, &oci_server, OCI_HTYPE_SERVER, (size_t) 0, NULL);
	if (errcode != 0)
	{
		printf("OCIHandleAlloc %d failed with errcode = %d.\n", OCI_HTYPE_SERVER, errcode);
		checkerr(oci_err, errcode);
		goto _exit_err;
	}

	errcode = OCIServerAttach(oci_server, oci_err, (text *) database, (sb4) strlen(database), 0);
	if (errcode != 0)
	{
		printf("OCIServerAttach failed with errcode = %d.\n", errcode);
		checkerr(oci_err, errcode);
		goto _exit_server;
	}

//	errcode = OCILogon2(oci_env, oci_err, &oci_service, username, strlen(username), password, strlen(password), database, strlen(database), OCI_DEFAULT);
	errcode = OCIHandleAlloc(oci_env, &oci_service, OCI_HTYPE_SVCCTX, (size_t) 0, NULL);
	if (errcode != 0)
	{
		printf("OCIHandleAlloc %d failed with errcode = %d.\n", OCI_HTYPE_SVCCTX, errcode);
		checkerr(oci_err, errcode);
		goto _exit_server;
	}
	
	errcode = OCIAttrSet(oci_service, OCI_HTYPE_SVCCTX, oci_server, (ub4) 0, OCI_ATTR_SERVER, (OCIError *) oci_err);
	if (errcode != 0)
	{
		printf("OCIAttrSet type %d failed with errcode = %d.\n", OCI_HTYPE_SVCCTX, errcode);
		checkerr(oci_err, errcode);
		goto _exit_service;
	}

	errcode = OCIHandleAlloc(oci_env, (dvoid **)&oci_auth, (ub4) OCI_HTYPE_SESSION, (size_t) 0, NULL);
	if (errcode != 0)
	{
		printf("OCIHandleAlloc type %d failed with errcode = %d.\n", OCI_HTYPE_SESSION, errcode);
		checkerr(oci_err, errcode);
		goto _exit_service;
	}

	errcode = OCIAttrSet(oci_auth, (ub4) OCI_HTYPE_SESSION, username, (ub4) strlen((char *)username), (ub4) OCI_ATTR_USERNAME, oci_err);
	if (errcode != 0)
	{
		printf("OCIAttrSet type %d username failed with errcode = %d.\n", OCI_HTYPE_SESSION, errcode);
		checkerr(oci_err, errcode);
		goto _exit_session;
	}

	errcode = OCIAttrSet(oci_auth, (ub4) OCI_HTYPE_SESSION, password, (ub4) strlen((char *)password), (ub4) OCI_ATTR_PASSWORD, oci_err);
	if (errcode != 0)
	{
		printf("OCIAttrSet type %d password failed with errcode = %d.\n", OCI_HTYPE_SESSION, errcode);
		checkerr(oci_err, errcode);
		goto _exit_session;
	}

	errcode = OCISessionBegin(oci_service,  oci_err, oci_auth, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT);
	if (errcode != 0)
	{
		printf("OCISessionBegin failed with errcode = %d.\n", errcode);
		checkerr(oci_err, errcode);
		goto _exit_session;
	}

	errcode = OCIAttrSet(oci_service, (ub4) OCI_HTYPE_SVCCTX, oci_auth, (ub4) 0, (ub4) OCI_ATTR_SESSION, oci_err);
	if (errcode != 0)
	{
		printf("OCIAttrSet type %d failed with errcode = %d.\n", OCI_ATTR_SESSION, errcode);
		checkerr(oci_err, errcode);
		goto _exit;
	}

	exec_stmt();

	main_ret_code = 0;

_exit:
	errcode = OCISessionEnd(oci_service, oci_err, oci_auth, (ub4) 0);
	if (errcode != 0)
	{
		printf("FAILED: OCISessionEnd()\n");
		checkerr(oci_err, errcode);
	}

	errcode = OCIServerDetach(oci_server, oci_err, (ub4) OCI_DEFAULT);
	if (errcode != 0)
	{
		printf("FAILED: OCIServerDetach()\n");
		checkerr(oci_err, errcode);
	}

_exit_session:
	errcode = OCIHandleFree(oci_auth, (ub4) OCI_HTYPE_SESSION);
	if (errcode != 0)
	{
		printf("OCIHandleFree type %d failed with errcode = %d.\n", OCI_HTYPE_SESSION, errcode);
		checkerr(oci_err, errcode);
	}

_exit_service:
	errcode = OCIHandleFree(oci_service, (ub4) OCI_HTYPE_SVCCTX);
	if (errcode != 0)
	{
		printf("OCIHandleFree type %d failed with errcode = %d.\n", OCI_HTYPE_SVCCTX, errcode);
		checkerr(oci_err, errcode);
	}

_exit_server:
	errcode = OCIHandleFree(oci_server, (ub4) OCI_HTYPE_SERVER);
	if (errcode != 0)
	{
		printf("OCIHandleFree type %d failed with errcode = %d.\n", OCI_HTYPE_SERVER, errcode);
		checkerr(oci_err, errcode);
	}

_exit_err:
	errcode = OCIHandleFree(oci_err, (ub4) OCI_HTYPE_ERROR);
	if (errcode != 0)
	{
		printf("OCIHandleFree type %d failed with errcode = %d.\n", OCI_HTYPE_ERROR, errcode);
		checkerr(oci_err, errcode);
	}

_exit_env:
	errcode = OCIHandleFree(oci_env, (ub4) OCI_HTYPE_ENV);
	if (errcode != 0)
	{
		printf("OCIHandleFree type %d failed with errcode = %d.\n", OCI_HTYPE_ENV, errcode);
	}

_exit_terminate:
	errcode = OCITerminate(OCI_DEFAULT);
	if (errcode != 0)
	{
		printf("OCITerminate failed with errcode = %d.\n", errcode);
	}

	return main_ret_code;
}
