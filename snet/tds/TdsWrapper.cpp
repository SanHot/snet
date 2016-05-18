//
// Created by San on 16/3/28.
//

#include "TdsWrapper.h"
#include <assert.h>

std::string s_error;

//==================================================================================

TdsQuery::TdsQuery(TdsWrapper* pTds):pStmt(NULL),m_eof(false),m_cols(0) {
    m_pTds = pTds;
}

TdsQuery::~TdsQuery() {
    finalize();
}

int TdsQuery::numFields() {
    return m_cols;
}

const char *TdsQuery::fieldName(int nCol) {
    if(pStmt && nCol < m_cols) {
        return pStmt[nCol].name;
    }
    return NULL;
}

const char *TdsQuery::fieldDeclType(int nCol) {
    return NULL;
}

int TdsQuery::fieldDataType(int nCol) {
    if(pStmt && nCol < m_cols) {
        return pStmt[nCol].type;
    }
    return -1;
}

const char *TdsQuery::fieldValue(int nField) {
    if(pStmt && nField < m_cols) {
        if(pStmt[nField].status == -1)
            pStmt[nField].buffer[0] = '\0';
        else
            pStmt[nField].buffer[pStmt[nField].size] = '\0';
        return pStmt[nField].buffer;
    }
    return NULL;
}

const char *TdsQuery::fieldValue(const char *szField) {
    return nullptr;
}

int TdsQuery::getIntField(int nField, int nNullValue) {
    return 0;
}

int TdsQuery::getIntField(const char *szField, int nNullValue) {
    return 0;
}

double TdsQuery::getFloatField(int nField, double fNullValue) {
    return 0;
}

double TdsQuery::getFloatField(const char *szField, double fNullValue) {
    return 0;
}

const char *TdsQuery::getStringField(int nField, const char *szNullValue) {
    return nullptr;
}

const char *TdsQuery::getStringField(const char *szField, const char *szNullValue) {
    return nullptr;
}

const unsigned char *TdsQuery::getBlobField(int nField, int &nLen) {
    return nullptr;
}

const unsigned char *TdsQuery::getBlobField(const char *szField, int &nLen) {
    return nullptr;
}

bool TdsQuery::fieldIsNull(int nField) {
    return false;
}

bool TdsQuery::fieldIsNull(const char *szField) {
    return false;
}

bool TdsQuery::eof() {
    return m_eof;
}

void TdsQuery::execQuery(const char *sql) {
    if(sql == NULL) {
        throw TdsException("sql_string: NULL");
    }

    this->finalize();
    m_pTds->execDML(sql);

    m_cols = dbnumcols(m_pTds->m_dbproc);
    pStmt = new TdsStmt[m_cols];
    for(int i = 0; i < m_cols; ++i) {
        pStmt[i].name = dbcolname(m_pTds->m_dbproc, i + 1);
        pStmt[i].type = dbcoltype(m_pTds->m_dbproc, i + 1);
        pStmt[i].size = dbcollen(m_pTds->m_dbproc, i + 1);
        //绑定转换
        if (SYBCHAR != pStmt[i].type) {
            pStmt[i].size  = dbprcollen(m_pTds->m_dbproc, i + 1);
            if (pStmt[i].size  > 255)
                pStmt[i].size  = 255;
        }
        //分配空间
        pStmt[i].buffer = new char[pStmt[i].size + 1];
        //开始绑定
        RETCODE ret = dbbind(m_pTds->m_dbproc, i+1, NTBSTRINGBIND,  pStmt[i].size + 1, (BYTE*)pStmt[i].buffer);
        if (ret == FAIL)
            throw TdsException("dbbind: FAIL");
        //绑定null状态
        ret = dbnullbind(m_pTds->m_dbproc, i + 1, &pStmt[i].status);
        if (ret == FAIL)
            throw TdsException("dbnullbind: FAIL");
    }

    this->nextRow();
}

int TdsQuery::execScalar(const char *szSQL) {
    execQuery(szSQL);
    if (eof() || numFields() < 1) {
        throw TdsException("Invalid scalar execQuery");
    }
    return atoi(fieldValue(0));
}

void TdsQuery::nextRow() {
    int row_code = dbnextrow(m_pTds->m_dbproc);
    if(row_code == NO_MORE_ROWS) {
        if (DBMORECMDS(m_pTds->m_dbproc) == SUCCEED) {
            while ((row_code =dbresults(m_pTds->m_dbproc)) != NO_MORE_RESULTS) {
                if (row_code == FAIL) {
                    dbcancel(m_pTds->m_dbproc);
                    throw TdsException("nextRow(): DBMORECMDS => FAIL");
                }
                if (DBCMDROW(m_pTds->m_dbproc) == SUCCEED) {
                    if (DBROWS(m_pTds->m_dbproc) == SUCCEED) {
                        m_eof = false;
                        return;
                    }
                }
            }
        }
        m_eof = true;
    }
    else  if(row_code == REG_ROW) {
        m_eof = false;
    }
    else if(row_code == BUF_FULL) {
        m_eof = true;
        throw TdsException("nextRow(): dbnextrow => BUF_FULL");
    }
    else {
        m_eof = true;
        dbcancel(m_pTds->m_dbproc);
        this->finalize();
        throw TdsException("nextRow(): dbnextrow => FAIL");
    }
}

void TdsQuery::finalize() {
    if(m_cols > 0) {
        for (int i = 0; i < m_cols; ++i) {
            delete pStmt[i].buffer;
        }
        delete[] pStmt;
        pStmt = NULL;
    }
    m_cols = 0;
}

//==================================================================================

TdsWrapper::TdsWrapper() :m_dbproc(NULL)
{
}

TdsWrapper::~TdsWrapper()
{
    close();
}

bool TdsWrapper::connect(const char *server,
                         const char *user,
                         const char *password,
                         const char *dbname,
                         const char *charset) {
    if(server == NULL) {
        printf("connect: server is null\n");
        return false;
    }

    m_error.clear();
    LOGINREC *login;
    if (!m_dbproc) {
        dbinit();
        dberrhandle(TdsWrapper::err_handler);
        dbmsghandle(TdsWrapper::msg_handler);

        login = dblogin();
        DBSETLUSER(login, user);
        DBSETLPWD(login, password);
        DBSETLCHARSET(login, charset?charset:"GBK");
        m_dbproc = dbopen(login, server);
        dbloginfree(login);
        if (hasError())
            throw TdsException(m_error.c_str());
    }

    if (dbname) {
        dbuse(m_dbproc, dbname);
        if (hasError())
            throw TdsException(m_error.c_str());
    }

//    execute("set textsize 2147483000");
    return true;
}

int TdsWrapper::close() {
    if (m_dbproc) {
        dbfreebuf(m_dbproc);
        dbclose(m_dbproc);
        m_dbproc = NULL;
    }
    return 0;
}

int TdsWrapper::execDML(const char *sql) {
    RETCODE ret;
    int retVal = 0;
    m_error.clear();

    if (!m_dbproc)
        throw TdsException("execDML(): called with no active connection");

    if (dbcmd(m_dbproc, sql) != SUCCEED) {
        dbcancel(m_dbproc);
        throw TdsException("execDML(): dbcmd => FAIL");
    }

    if (dbsqlexec(m_dbproc) != SUCCEED) {
        dbcancel(m_dbproc);
        throw TdsException("execDML(): dbsqlexec => FAIL");
    }

    while ((ret = dbresults(m_dbproc)) != NO_MORE_RESULTS) {
        if (ret == FAIL) {
            dbcancel(m_dbproc);
            throw TdsException("execDML(): dbcancel => FAIL");
        }
        //有数据返回(针对select)
        if (DBCMDROW(m_dbproc) == SUCCEED) {
            // And it does return data
            if (DBROWS(m_dbproc) == SUCCEED)
                return 0;
            // It doesn't return data
            else
                return -1;
        }
        //无数据返回(针对insert,update,delete)
        else {
            retVal += DBCOUNT(m_dbproc);
        }
    }
    return retVal;
}

std::string TdsWrapper::fetchRow(const char *delimit_char) {
    if (dbnextrow(m_dbproc) == NO_MORE_ROWS) {
        return "";
    }
    std::string retval = "";
    int valLen = 0;
    DBCHAR tmp[65535] = {0};
    int colCount = dbnumcols(m_dbproc);
    for (int i = 1; i <= colCount; i++) {
        valLen = dbdatlen(m_dbproc, i);
        if (i > 1)
            retval += delimit_char;
        if (valLen > 0) {
            dbconvert(m_dbproc, dbcoltype(m_dbproc, i),  dbdata(m_dbproc, i), valLen, SYBCHAR, (BYTE *)tmp, (DBINT) -1);
            retval += tmp;
            //memset(tmp, 0x0, valLen);
        }
    }
    return retval;
}

int TdsWrapper::err_handler(DBPROCESS * dbproc, int severity, int dberr, int oserr,
            char *dberrstr, char *oserrstr) {
    if (dberr == SYBEICONVI)
        return INT_CANCEL;
    if (dberr == SYBESMSG)
        return INT_CANCEL;
    if (dberr) {
        fprintf(stderr,"DBLIB ERROR: [%s]\n", dberrstr);
    }
    else {
        fprintf(stderr,"OS    ERROR: [%s]\n", oserrstr);
    }
    if (dberr == SYBETIME)
        return INT_TIMEOUT;
    return INT_CANCEL;
}

int TdsWrapper::msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
            char *msgtext, char *srvname, char *procname, int line) {
    enum {changed_database = 5701, changed_language = 5703 };

    if (msgno == changed_database || msgno == changed_language)
        return 0;

    if (msgno > 0) {
        fprintf(stderr, "Msg %ld, Level %d, State %d\n",
                (long) msgno, severity, msgstate);

        if (strlen(srvname) > 0)
            fprintf(stderr, "Server '%s', ", srvname);
        if (strlen(procname) > 0)
            fprintf(stderr, "Procedure '%s', ", procname);
        if (line > 0)
            fprintf(stderr, "Line %d", line);

        fprintf(stderr, "\n\t");
    }
    fprintf(stderr, "%s\n", msgtext);

    if (severity > 10) {
        fprintf(stderr, "msg_handler: error: severity %d > 10, exiting\n", severity);
    }

    return 0;
}

int TdsWrapper::setMaxBuffer(int max) {
    //设置缓存
    char bufLen[16] = {0};
    sprintf(bufLen, "%d", max);
    if(dbsetopt(m_dbproc,DBBUFFER,bufLen,sizeof(bufLen)) == FAIL) {
//        printf("dbsetopt: error(%d)\n", max);
        return -1;
    }
    return 0;
}
















































