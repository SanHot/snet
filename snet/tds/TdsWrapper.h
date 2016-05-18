//
// Created by San on 16/3/28.
//

#ifndef TDSPP_TDSPP_H
#define TDSPP_TDSPP_H

#include <string>
#include <exception>
#include <sqlfront.h>
#include <sqldb.h>

#define MAX_ROWS 1000
#define ERR_MSG_LEN 1024

struct TdsStmt {
    char *name;
    char *buffer;
    int type, size, status;
};

class TdsException: public std::exception
{
public:
    explicit TdsException (const char* msg) {
        errMsg.assign(msg);
    }
    virtual const char* what() const throw() {
        return errMsg.c_str();
    }

private:
    std::string errMsg;
};

class TdsWrapper;
class TdsQuery
{
public:
    TdsQuery(TdsWrapper* pTds);
    virtual ~TdsQuery();

    int numFields();

    const char* fieldName(int nCol);
    const char* fieldDeclType(int nCol);

    int fieldDataType(int nCol);

    const char* fieldValue(int nField);
    const char* fieldValue(const char* szField);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(const char* szField, int nNullValue=0);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(const char* szField, double fNullValue=0.0);

    const char* getStringField(int nField, const char* szNullValue="");
    const char* getStringField(const char* szField, const char* szNullValue="");

    const unsigned char* getBlobField(int nField, int& nLen);
    const unsigned char* getBlobField(const char* szField, int& nLen);

    bool fieldIsNull(int nField);
    bool fieldIsNull(const char* szField);

    bool eof();
    void execQuery(const char *sql);
    int execScalar(const char* szSQL);
    void nextRow();
    void finalize();

    std::string operator[](const char *);
    std::string operator[](std::string);

private:
    bool m_eof;
    int m_cols;
    TdsStmt* pStmt;
    TdsWrapper* m_pTds;

};

class TdsWrapper
{
    friend class TdsQuery;
public:
    TdsWrapper();
    virtual ~TdsWrapper();
    virtual bool connect(const char *server, const char *user, const char *password, const char *dbname,
                 const char *charset = NULL);
    virtual int close();
    virtual int execDML(const char *sql);
    virtual std::string fetchRow(const char *delimit_char);

    virtual bool hasError() { return !m_error.empty(); };
    static int err_handler(DBPROCESS * dbproc, int severity, int dberr, int oserr,
                        char *dberrstr, char *oserrstr);
    static int msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
                           char *msgtext, char *srvname, char *procname, int line);

    int setMaxBuffer(int max);
private:
    TdsWrapper(const TdsWrapper&);
    TdsWrapper& operator=(const TdsWrapper&);

    DBPROCESS* m_dbproc;
    std::string m_error;
};


#endif //TDSPP_TDSPP_H
