#pragma once

#include "Windows.h"

#include "spy_types.h"

#define SQLITE_OK 0 /* Successful result */

/* beginning-of-error-codes */
#define SQLITE_ERROR      1   /* Generic error */
#define SQLITE_INTERNAL   2   /* Internal logic error in SQLite */
#define SQLITE_PERM       3   /* Access permission denied */
#define SQLITE_ABORT      4   /* Callback routine requested an abort */
#define SQLITE_BUSY       5   /* The database file is locked */
#define SQLITE_LOCKED     6   /* A table in the database is locked */
#define SQLITE_NOMEM      7   /* A malloc() failed */
#define SQLITE_READONLY   8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT  9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR      10  /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT    11  /* The database disk image is malformed */
#define SQLITE_NOTFOUND   12  /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL       13  /* Insertion failed because database is full */
#define SQLITE_CANTOPEN   14  /* Unable to open the database file */
#define SQLITE_PROTOCOL   15  /* Database lock protocol error */
#define SQLITE_EMPTY      16  /* Internal use only */
#define SQLITE_SCHEMA     17  /* The database schema changed */
#define SQLITE_TOOBIG     18  /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT 19  /* Abort due to constraint violation */
#define SQLITE_MISMATCH   20  /* Data type mismatch */
#define SQLITE_MISUSE     21  /* Library used incorrectly */
#define SQLITE_NOLFS      22  /* Uses OS features not supported on host */
#define SQLITE_AUTH       23  /* Authorization denied */
#define SQLITE_FORMAT     24  /* Not used */
#define SQLITE_RANGE      25  /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB     26  /* File opened that is not a database file */
#define SQLITE_NOTICE     27  /* Notifications from sqlite3_log() */
#define SQLITE_WARNING    28  /* Warnings from sqlite3_log() */
#define SQLITE_ROW        100 /* sqlite3_step() has another row ready */
#define SQLITE_DONE       101 /* sqlite3_step() has finished executing */
/* end-of-error-codes */

/*
** CAPI3REF: Extended Result Codes
** KEYWORDS: {extended result code definitions}
**
** In its default configuration, SQLite API routines return one of 30 integer
** [result codes].  However, experience has shown that many of
** these result codes are too coarse-grained.  They do not provide as
** much information about problems as programmers might like.  In an effort to
** address this, newer versions of SQLite (version 3.3.8 [dateof:3.3.8]
** and later) include
** support for additional result codes that provide more detailed information
** about errors. These [extended result codes] are enabled or disabled
** on a per database connection basis using the
** [sqlite3_extended_result_codes()] API.  Or, the extended code for
** the most recent error can be obtained using
** [sqlite3_extended_errcode()].
*/
#define SQLITE_ERROR_MISSING_COLLSEQ   (SQLITE_ERROR | (1 << 8))
#define SQLITE_ERROR_RETRY             (SQLITE_ERROR | (2 << 8))
#define SQLITE_ERROR_SNAPSHOT          (SQLITE_ERROR | (3 << 8))
#define SQLITE_IOERR_READ              (SQLITE_IOERR | (1 << 8))
#define SQLITE_IOERR_SHORT_READ        (SQLITE_IOERR | (2 << 8))
#define SQLITE_IOERR_WRITE             (SQLITE_IOERR | (3 << 8))
#define SQLITE_IOERR_FSYNC             (SQLITE_IOERR | (4 << 8))
#define SQLITE_IOERR_DIR_FSYNC         (SQLITE_IOERR | (5 << 8))
#define SQLITE_IOERR_TRUNCATE          (SQLITE_IOERR | (6 << 8))
#define SQLITE_IOERR_FSTAT             (SQLITE_IOERR | (7 << 8))
#define SQLITE_IOERR_UNLOCK            (SQLITE_IOERR | (8 << 8))
#define SQLITE_IOERR_RDLOCK            (SQLITE_IOERR | (9 << 8))
#define SQLITE_IOERR_DELETE            (SQLITE_IOERR | (10 << 8))
#define SQLITE_IOERR_BLOCKED           (SQLITE_IOERR | (11 << 8))
#define SQLITE_IOERR_NOMEM             (SQLITE_IOERR | (12 << 8))
#define SQLITE_IOERR_ACCESS            (SQLITE_IOERR | (13 << 8))
#define SQLITE_IOERR_CHECKRESERVEDLOCK (SQLITE_IOERR | (14 << 8))
#define SQLITE_IOERR_LOCK              (SQLITE_IOERR | (15 << 8))
#define SQLITE_IOERR_CLOSE             (SQLITE_IOERR | (16 << 8))
#define SQLITE_IOERR_DIR_CLOSE         (SQLITE_IOERR | (17 << 8))
#define SQLITE_IOERR_SHMOPEN           (SQLITE_IOERR | (18 << 8))
#define SQLITE_IOERR_SHMSIZE           (SQLITE_IOERR | (19 << 8))
#define SQLITE_IOERR_SHMLOCK           (SQLITE_IOERR | (20 << 8))
#define SQLITE_IOERR_SHMMAP            (SQLITE_IOERR | (21 << 8))
#define SQLITE_IOERR_SEEK              (SQLITE_IOERR | (22 << 8))
#define SQLITE_IOERR_DELETE_NOENT      (SQLITE_IOERR | (23 << 8))
#define SQLITE_IOERR_MMAP              (SQLITE_IOERR | (24 << 8))
#define SQLITE_IOERR_GETTEMPPATH       (SQLITE_IOERR | (25 << 8))
#define SQLITE_IOERR_CONVPATH          (SQLITE_IOERR | (26 << 8))
#define SQLITE_IOERR_VNODE             (SQLITE_IOERR | (27 << 8))
#define SQLITE_IOERR_AUTH              (SQLITE_IOERR | (28 << 8))
#define SQLITE_IOERR_BEGIN_ATOMIC      (SQLITE_IOERR | (29 << 8))
#define SQLITE_IOERR_COMMIT_ATOMIC     (SQLITE_IOERR | (30 << 8))
#define SQLITE_IOERR_ROLLBACK_ATOMIC   (SQLITE_IOERR | (31 << 8))
#define SQLITE_IOERR_DATA              (SQLITE_IOERR | (32 << 8))
#define SQLITE_IOERR_CORRUPTFS         (SQLITE_IOERR | (33 << 8))
#define SQLITE_LOCKED_SHAREDCACHE      (SQLITE_LOCKED | (1 << 8))
#define SQLITE_LOCKED_VTAB             (SQLITE_LOCKED | (2 << 8))
#define SQLITE_BUSY_RECOVERY           (SQLITE_BUSY | (1 << 8))
#define SQLITE_BUSY_SNAPSHOT           (SQLITE_BUSY | (2 << 8))
#define SQLITE_BUSY_TIMEOUT            (SQLITE_BUSY | (3 << 8))
#define SQLITE_CANTOPEN_NOTEMPDIR      (SQLITE_CANTOPEN | (1 << 8))
#define SQLITE_CANTOPEN_ISDIR          (SQLITE_CANTOPEN | (2 << 8))
#define SQLITE_CANTOPEN_FULLPATH       (SQLITE_CANTOPEN | (3 << 8))
#define SQLITE_CANTOPEN_CONVPATH       (SQLITE_CANTOPEN | (4 << 8))
#define SQLITE_CANTOPEN_DIRTYWAL       (SQLITE_CANTOPEN | (5 << 8)) /* Not Used */
#define SQLITE_CANTOPEN_SYMLINK        (SQLITE_CANTOPEN | (6 << 8))
#define SQLITE_CORRUPT_VTAB            (SQLITE_CORRUPT | (1 << 8))
#define SQLITE_CORRUPT_SEQUENCE        (SQLITE_CORRUPT | (2 << 8))
#define SQLITE_CORRUPT_INDEX           (SQLITE_CORRUPT | (3 << 8))
#define SQLITE_READONLY_RECOVERY       (SQLITE_READONLY | (1 << 8))
#define SQLITE_READONLY_CANTLOCK       (SQLITE_READONLY | (2 << 8))
#define SQLITE_READONLY_ROLLBACK       (SQLITE_READONLY | (3 << 8))
#define SQLITE_READONLY_DBMOVED        (SQLITE_READONLY | (4 << 8))
#define SQLITE_READONLY_CANTINIT       (SQLITE_READONLY | (5 << 8))
#define SQLITE_READONLY_DIRECTORY      (SQLITE_READONLY | (6 << 8))
#define SQLITE_ABORT_ROLLBACK          (SQLITE_ABORT | (2 << 8))
#define SQLITE_CONSTRAINT_CHECK        (SQLITE_CONSTRAINT | (1 << 8))
#define SQLITE_CONSTRAINT_COMMITHOOK   (SQLITE_CONSTRAINT | (2 << 8))
#define SQLITE_CONSTRAINT_FOREIGNKEY   (SQLITE_CONSTRAINT | (3 << 8))
#define SQLITE_CONSTRAINT_FUNCTION     (SQLITE_CONSTRAINT | (4 << 8))
#define SQLITE_CONSTRAINT_NOTNULL      (SQLITE_CONSTRAINT | (5 << 8))
#define SQLITE_CONSTRAINT_PRIMARYKEY   (SQLITE_CONSTRAINT | (6 << 8))
#define SQLITE_CONSTRAINT_TRIGGER      (SQLITE_CONSTRAINT | (7 << 8))
#define SQLITE_CONSTRAINT_UNIQUE       (SQLITE_CONSTRAINT | (8 << 8))
#define SQLITE_CONSTRAINT_VTAB         (SQLITE_CONSTRAINT | (9 << 8))
#define SQLITE_CONSTRAINT_ROWID        (SQLITE_CONSTRAINT | (10 << 8))
#define SQLITE_CONSTRAINT_PINNED       (SQLITE_CONSTRAINT | (11 << 8))
#define SQLITE_CONSTRAINT_DATATYPE     (SQLITE_CONSTRAINT | (12 << 8))
#define SQLITE_NOTICE_RECOVER_WAL      (SQLITE_NOTICE | (1 << 8))
#define SQLITE_NOTICE_RECOVER_ROLLBACK (SQLITE_NOTICE | (2 << 8))
#define SQLITE_WARNING_AUTOINDEX       (SQLITE_WARNING | (1 << 8))
#define SQLITE_AUTH_USER               (SQLITE_AUTH | (1 << 8))
#define SQLITE_OK_LOAD_PERMANENTLY     (SQLITE_OK | (1 << 8))
#define SQLITE_OK_SYMLINK              (SQLITE_OK | (2 << 8)) /* internal use only */

#define SQLITE_INTEGER 1
#define SQLITE_FLOAT   2
#define SQLITE_BLOB    4
#define SQLITE_NULL    5
#define SQLITE_TEXT    3

#define SQLITE3_EXEC_OFFSET             0x3A5EDA0
#define SQLITE3_BACKUP_INIT_OFFSET      0x3A18EA0
#define SQLITE3_PREPARE_OFFSET          0x3A66A20
#define SQLITE3_OPEN_OFFSET             0x3A9E210
#define SQLITE3_BACKUP_STEP_OFFSET      0x3A193F0
#define SQLITE3_BACKUP_REMAINING_OFFSET 0x1B26EB0
#define SQLITE3_BACKUP_PAGECOUNT_OFFSET 0x1B26EE0
#define SQLITE3_BACKUP_FINISH_OFFSET    0x3A19AF0
#define SQLITE3_SLEEP_OFFSET            0x3A9EE70
#define SQLITE3_ERRCODE_OFFSET          0x3A9CB10
#define SQLITE3_CLOSE_OFFSET            0x3A9AC70
#define SQLITE3_STEP_OFFSET             0x3A22DA0
#define SQLITE3_COLUMN_COUNT_OFFSET     0x3A235C0
#define SQLITE3_COLUMN_NAME_OFFSET      0x3A23FC0
#define SQLITE3_COLUMN_TYPE_OFFSET      0x3A23E10
#define SQLITE3_COLUMN_BLOB_OFFSET      0x3A235F0
#define SQLITE3_COLUMN_BYTES_OFFSET     0x3A236E0
#define SQLITE3_FINALIZE_OFFSET         0x3A21E50

typedef int (*Sqlite3_callback)(void *, int, char **, char **);

typedef int(__cdecl *Sqlite3_exec)(QWORD,            /* An open database */
                                   const char *sql,  /* SQL to be evaluated */
                                   Sqlite3_callback, /* Callback function */
                                   void *,           /* 1st argument to callback */
                                   char **errmsg     /* Error msg written here */
);
typedef QWORD(__cdecl *Sqlite3_backup_init)(QWORD *pDest,           /* Destination database handle */
                                            const char *zDestName,  /* Destination database name */
                                            QWORD *pSource,         /* Source database handle */
                                            const char *zSourceName /* Source database name */
);
typedef int(__cdecl *Sqlite3_prepare)(QWORD db,           /* Database handle */
                                      const char *zSql,   /* SQL statement, UTF-8 encoded */
                                      int nByte,          /* Maximum length of zSql in bytes. */
                                      QWORD **ppStmt,     /* OUT: Statement handle */
                                      const char **pzTail /* OUT: Pointer to unused portion of zSql */
);
typedef int(__cdecl *Sqlite3_open)(const char *filename, QWORD **ppDb);
typedef int(__cdecl *Sqlite3_backup_step)(QWORD *p, int nPage);
typedef int(__cdecl *Sqlite3_backup_remaining)(QWORD *p);
typedef int(__cdecl *Sqlite3_backup_pagecount)(QWORD *p);
typedef int(__cdecl *Sqlite3_backup_finish)(QWORD *p);
typedef int(__cdecl *Sqlite3_sleep)(int);
typedef int(__cdecl *Sqlite3_errcode)(QWORD *db);
typedef int(__cdecl *Sqlite3_close)(QWORD *);

typedef int(__cdecl *Sqlite3_step)(QWORD *);
typedef int(__cdecl *Sqlite3_column_count)(QWORD *pStmt);
typedef const char *(__cdecl *Sqlite3_column_name)(QWORD *, int N);
typedef int(__cdecl *Sqlite3_column_type)(QWORD *, int iCol);
typedef const void *(__cdecl *Sqlite3_column_blob)(QWORD *, int iCol);
typedef int(__cdecl *Sqlite3_column_bytes)(QWORD *, int iCol);
typedef int(__cdecl *Sqlite3_finalize)(QWORD *pStmt);
