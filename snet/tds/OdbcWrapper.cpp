//
// Created by san on 16/5/18.
//

#include "OdbcWrapper.h"

#define MAX_BUFFER_SIZE 256

OdbcQuery::OdbcQuery(OdbcWrapper *pTds) {

}

OdbcQuery::~TdsQuery() {

}

int OdbcQuery::numFields() {
    return 0;
}

const char *OdbcQuery::fieldName(int nCol) {
    return nullptr;
}

const char *OdbcQuery::fieldDeclType(int nCol) {
    return nullptr;
}

int OdbcQuery::fieldDataType(int nCol) {
    return 0;
}

const char *OdbcQuery::fieldValue(int nField) {
    return nullptr;
}

bool OdbcQuery::eof() {
    return false;
}

void OdbcQuery::execQuery(const char *sql) {
    pOdbc->execDML(sql);

    SQLSMALLINT cols = 0;
    struct COL {
        char *name;
        int name_size;
        char *buffer;
        int buffer_size;
        SQLSMALLINT dataType;
    } *pCol;

    SQLNumResultCols(pOdbc->sqlstatementhandle, &cols);
    pCol = new COL[cols];
    for (int i = 0; i < cols; i++) {
        pCol[i].name = new char[128];
        SQLColAttribute(pOdbc->sqlstatementhandle, (SQLUSMALLINT) i + 1, SQL_DESC_LABEL,
                        pCol[i].name, 128, (SQLSMALLINT *) &pCol[i].name_size, NULL);

        SQLColAttribute(pOdbc->sqlstatementhandle, (SQLUSMALLINT) i + 1, SQL_COLUMN_TYPE,
                        NULL, 0, NULL, (SQLSMALLINT *) &pCol[i].dataType);

        pCol[i].buffer = new char[MAX_BUFFER_SIZE];
        SQLBindCol(pOdbc->sqlstatementhandle, (SQLUSMALLINT) i + 1, SQL_C_CHAR, pCol[i].buffer, MAX_BUFFER_SIZE,
                   (SQLLEN *) &pCol[i].buffer_size);
//        printf("%s\t", pCol[i].name);
    }
//    printf("\n");
}

int OdbcQuery::execScalar(const char *szSQL) {
    return 0;
}

void OdbcQuery::nextRow() {

}

void OdbcQuery::finalize() {

}


OdbcWrapper::OdbcWrapper() {

}

OdbcWrapper::~OdbcWrapper() {

}

bool OdbcWrapper::connect(const char *server, const char *user, const char *password, const char *dbname,
                          const char *charset) {

    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle)) {
        close();
        return false;
    }
    if (SQL_SUCCESS != SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0)) {
        close();
        return false;
    }
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle)) {
        close();
        return false;
    }

    SQLCHAR retconstring[1024];
    char addr[1024] = {0};
    sprintf(addr, "DRIVER={SQL Server};SERVER=%s;DATABASE=%s;UID=%s;PWD=%s;",server,dbname,user,password);
    switch (SQLDriverConnectA(sqlconnectionhandle,
                              NULL,
                              (SQLCHAR*) addr,
                              SQL_NTS,
                              retconstring,
                              1024,
                              NULL,
                              SQL_DRIVER_NOPROMPT)) {
        case SQL_SUCCESS_WITH_INFO:
            get_error(SQL_HANDLE_DBC, sqlconnectionhandle);
            break;
        case SQL_INVALID_HANDLE:
        case SQL_ERROR:
            get_error(SQL_HANDLE_DBC, sqlconnectionhandle);
            close();
            return false;
        default:
            break;
    }
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle)) {
        close();
        return false;
    }
    return true;
}

int OdbcWrapper::close() {
    SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
    SQLDisconnect(sqlconnectionhandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
    return 0;
}

int OdbcWrapper::execDML(const char *sql) {
    if (SQL_SUCCESS != SQLExecDirectA(sqlstatementhandle, (SQLCHAR *) sql, SQL_NTS)) {
        get_error(SQL_HANDLE_STMT, sqlstatementhandle);
        SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
        return -1;
    }
    return 0;
}

std::string OdbcWrapper::fetchRow(const char *delimit_char) {

    return "";
}

void OdbcWrapper::get_error(unsigned int handletype, const SQLHANDLE &handle) {
    SQLCHAR sqlstate[1024];
    SQLCHAR message[1024];
    char err[2048] = {0};
    if (SQL_SUCCESS == SQLGetDiagRecA(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL)) {
        sprintf(err,"Message: %s\nSQLSTATE: %s", message, sqlstate);
        m_error.assign(err);
    }
}



















