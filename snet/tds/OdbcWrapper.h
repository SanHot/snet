//
// Created by san on 16/5/18.
//

#ifndef SNET_ODBCWRAPPER_H
#define SNET_ODBCWRAPPER_H

#include <exception>
#include <string>
#include <sql.h>
#include <sqlext.h>

struct OdbcStmt {
    char *name;
    char *buffer;
    int type, size, status;
};

class OdbcException: public std::exception
{
public:
    explicit OdbcException (const char* msg) {
        errMsg.assign(msg);
    }
    virtual const char* what() const throw() {
        return errMsg.c_str();
    }

private:
    std::string errMsg;
};

class OdbcWrapper;
class OdbcQuery {
public:
    OdbcQuery(OdbcWrapper* pTds);
    virtual ~OdbcQuery();

    int numFields();

    const char* fieldName(int nCol);
    const char* fieldDeclType(int nCol);
    int fieldDataType(int nCol);
    const char* fieldValue(int nField);

    bool eof();
    void execQuery(const char *sql);
    int execScalar(const char* szSQL);
    void nextRow();
    void finalize();

private:
    OdbcWrapper* pOdbc;
};

class OdbcWrapper {
    friend class OdbcQuery;
public:
    OdbcWrapper();
    virtual ~OdbcWrapper();
    virtual bool connect(const char *server, const char *user, const char *password, const char *dbname,
                        const char *charset = NULL);
    virtual int close();
    virtual int execDML(const char *sql);
    virtual std::string fetchRow(const char *delimit_char);

private:
    OdbcWrapper(const OdbcWrapper&);
    OdbcWrapper& operator=(const OdbcWrapper&);
    void get_error(unsigned int handletype, const SQLHANDLE &handle);

    SQLHANDLE sqlenvhandle;
    SQLHANDLE sqlconnectionhandle = 0;
    SQLHANDLE sqlstatementhandle = 0;

    std::string m_error;
};


#endif //SNET_ODBCWRAPPER_H
