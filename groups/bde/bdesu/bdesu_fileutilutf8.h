// bdesu_fileutilutf8.h                                               -*-C++-*-
#ifndef INCLUDED_BDESU_FILEUTILUTF8
#define INCLUDED_BDESU_FILEUTILUTF8

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a set of portable utilities for file system access.
//
//@CLASSES:
//  bdesu_FileUtilUtf8: namespace for file system acess functions
//
//@SEE_ALSO: bdesu_fileutil, bdesu_pathutil
//
//@AUTHOR: Andrei Basov (abasov), Oleg Semenov (osemenov)
//
//@DESCRIPTION: This component provides a platform-independent interface to
// filesystem utility methods.
//
///Platform-Specific File Locking Caveats
///--------------------------------------
// Locking has the following caveats for the following operating systems:
//:
//: o On Posix, closing a file releases all locks on all file descriptors
//:   referring to that file within the current process.  [Doc 1] [Doc 2]
//:
//: o On Posix, the child of a fork does not inherit the locks of the parent
//:   process.  [Doc 1] [Doc 2]
//:
//: o On at least some flavors of Unix, you can't lock a file for writing using
//:   a file descriptor opened in read-only mode.
//
///Platform-Specific Atomicity Caveats
///-----------------------------------
// The 'bdesu_FileUtilUtf8::read' and 'bdesu_FileUtilUtf8::write' methods add
// no atomicity guarantees for reading and writing to those provided (if any)
// by the underlying platform's methods for reading and writing (see
// 'http://lwn.net/Articles/180387/').
//
///Platform-Specific File Name Encoding Caveats
///--------------------------------------------
// File-name encodings have the following caveats for the following operating
// systems:
//
//: o On Windows, methods of 'bdesu_FileUtilUtf8' that take a file or directory
//:   name or pattern as a 'char*' or 'bsl::string' type assume that the name
//:   is encoded in UTF-8.  The routines attempt to convert the name to a
//:   UTF-16 'wchar_t' string via 'bdede_CharConvertUtf16::utf8ToUtf16', and if
//:   the conversion succeeds, call the Windows wide-character 'W' APIs with
//:   the UTF-16 name.  If the conversion fails, the methods fail.  Similarly,
//:   file searches returning file names call the Windows wide-character 'W'
//:   APIs and convert the resulting UTF-16 names to UTF-8.
//:
//:   Narrow-character file names in other encodings, containing characters
//:   with values in the range 128 - 255, will likely result in files being
//:   created with names that appear garbled.
//:
//:   Neither 'utf8ToUtf16' nor the Windows 'W' APIs do any normalization of
//:   the UTF-16 strings resulting from UTF-8 conversion, and it is therefore
//:   possible to have sets of file names which display as identical strings
//:   but are treated as different names by the file system.
//:
//: o On Posix, a file name or pattern supplied to methods of
//:   'bdesu_FileUtilUtf8' as a 'char*' or 'bsl::string' type are assumed to be
//:   encoded in UTF-8, and are passed unchanged to the underlying system file
//:   APIs, which are assumed to be interfacing with a filesystem encoded in
//:   UTF-8.  Because the file names and patterns are passed unchanged,
//:   'bdesu_FileUtilUtf8' methods will work correctly on Posix with other
//:   encodings, providing that the strings supplied to the methods are in the
//:   same encoding as the underlying file system.
//
///Platform-Specific File Open Mode Caveats
///----------------------------------------
// The 'bdesu_FileUtilUtf8::open' method adds no guarantees for the validity of
// combinations file open modes to those provided (if any) by the underlying
// platform's methods for opening files.  In particular, on Windows platforms,
// 'open' will fail with a combination of file open modes that requests
// truncation of an existing file without providing permission to either create
// a new file or overwrite the existing file.  The affected file open modes are
// listed below and should be avoided in applications that are intended to
// behave identically on both Windows and Posix platforms:
//
//: o e_FILE_OPEN, e_IO_READ_ONLY  , e_INIT_TRUNCATE
//: o e_FILE_OPEN, e_IO_APPEND_ONLY, e_INIT_TRUNCATE
//: o e_FILE_OPEN, e_IO_READ_APPEND, e_INIT_TRUNCATE
//
///Usage
///-----
///Example 1: General Usage
///- - - - - - - - - - - - -
// In this example, we start with a (relative) native path to a directory
// containing log files:
//..
//  #ifdef BSLS_PLATFORM_OS_WINDOWS
//    bsl::string logPath = "temp.1\\logs";
//  #else
//    bsl::string logPath = "temp.1/logs";
//  #endif
//..
// Suppose that we want to separate files into "old" and "new" subdirectories
// on the basis of modification time.  We will provide paths representing these
// locations, and create the directories if they do not exist:
//..
//  bsl::string oldPath(logPath), newPath(logPath);
//  bdesu_PathUtil::appendRaw(&oldPath, "old");
//  bdesu_PathUtil::appendRaw(&newPath, "new");
//  int rc = bdesu_FileUtilUtf8::createDirectories(oldPath.c_str(), true);
//  assert(0 == rc);
//  rc = bdesu_FileUtilUtf8::createDirectories(newPath.c_str(), true);
//  assert(0 == rc);
//..
// We know that all of our log files match the pattern "*.log", so let's search
// for all such files in the log directory:
//..
//  bdesu_PathUtil::appendRaw(&logPath, "*.log");
//  bsl::vector<bsl::string> logFiles;
//  bdesu_FileUtilUtf8::findMatchingPaths(&logFiles, logPath.c_str());
//..
// Now for each of these files, we will get the modification time.  Files that
// are older than 2 days will be moved to "old", and the rest will be moved to
// "new":
//..
//  bdet_Datetime modTime;
//  bsl::string   fileName;
//  for (bsl::vector<bsl::string>::iterator it = logFiles.begin();
//                                                it != logFiles.end(); ++it) {
//    assert(0 == bdesu_FileUtilUtf8::getLastModificationTime(&modTime, *it));
//    assert(0 == bdesu_PathUtil::getLeaf(&fileName, *it));
//    bsl::string *whichDirectory =
//                2 < (bdetu_SystemTime::nowAsDatetime() - modTime).totalDays()
//                ? &oldPath
//                : &newPath;
//    bdesu_PathUtil::appendRaw(whichDirectory, fileName.c_str());
//    assert(0 == bdesu_FileUtilUtf8::move(it->c_str(),
//                                         whichDirectory->c_str()));
//    bdesu_PathUtil::popLeaf(whichDirectory);
//  }
//..
///Example 2: Using 'bdesu_FileUtilUtf8::visitPaths'
/// - - - - - - - - - - - - - - - - - - - - - - - - -
// 'bdesu_FileUtilUtf8::visitPaths' enables clients to define a functor to
// operate on file paths that match a specified pattern.  In this example, we
// create a function that can be used to filter out files that have a last
// modified time within a particular time frame.
//
// First we define our filtering function:
//..
//  void getFilesWithinTimeframe(bsl::vector<bsl::string> *vector,
//                               const char               *item,
//                               const bdet_Datetime&      start,
//                               const bdet_Datetime&      end)
//  {
//      bdet_Datetime datetime;
//      int ret = bdesu_FileUtilUtf8::getLastModificationTime(&datetime,
//                                                             item);
//
//      if (ret) {
//          return;                                                   // RETURN
//      }
//
//      if (datetime < start || datetime > end) {
//          return;                                                   // RETURN
//      }
//
//      vector->push_back(item);
//  }
//..
// Then, with the help of 'bdesu_FileUtilUtf8::visitPaths' and
// 'bdef_BindUtil::bind', we create a function for finding all file paths that
// match a specified pattern and have a last modified time within a specified
// start and end time (both specified as a 'bdet_Datetime'):
//..
//  void findMatchingFilesInTimeframe(bsl::vector<bsl::string> *result,
//                                    const char               *pattern,
//                                    const bdet_Datetime&      start,
//                                    const bdet_Datetime&      end)
//  {
//      result->clear();
//      bdesu_FileUtilUtf8::visitPaths(
//                                pattern,
//                                bdef_BindUtil::bind(&getFilesWithinTimeframe,
//                                                    result,
//                                                    bdef_PlaceHolders::_1,
//                                                    start,
//                                                    end));
//  }
//..

#ifndef INCLUDED_BDESCM_VERSION
#include <bdescm_version.h>
#endif

#ifndef INCLUDED_BDESU_MEMORYUTIL
#include <bdesu_memoryutil.h>
#endif

#ifndef INCLUDED_BDEF_FUNCTION
#include <bdef_function.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

#ifndef INCLUDED_BSL_STRING
#include <bsl_string.h>
#endif

#ifndef INCLUDED_BSL_VECTOR
#include <bsl_vector.h>
#endif

#ifndef INCLUDED_BSL_CSTDDEF
#include <bsl_cstddef.h>
#endif

#ifndef BSLS_PLATFORM_OS_WINDOWS
#ifndef INCLUDED_SYS_TYPES
#include <sys/types.h>
#define INCLUDED_SYS_TYPES
#endif
#endif

namespace BloombergLP {

class bdet_Datetime;

                         // =========================
                         // struct bdesu_FileUtilUtf8
                         // =========================

struct bdesu_FileUtilUtf8 {
    // The static methods of this structure provide platform-independent
    // mechanisms for file system access.

    // TYPES
#ifdef BSLS_PLATFORM_OS_WINDOWS
    typedef void *HANDLE;
    typedef HANDLE FileDescriptor;
    typedef __int64 Offset;
    static const Offset OFFSET_MAX = _I64_MAX;
    static const Offset OFFSET_MIN = _I64_MIN;
#else
    typedef int     FileDescriptor;
#if defined(BSLS_PLATFORM_OS_FREEBSD) \
 || defined(BSLS_PLATFORM_OS_DARWIN)  \
 || defined(BSLS_PLATFORM_OS_CYGWIN)
    // 'off_t' is 64-bit on Darwin/FreeBSD/cygwin (even when running 32-bit),
    // so they do not have an 'off64_t' type.

    typedef off_t Offset;
#else
    typedef off64_t Offset;
#endif
#ifdef BSLS_PLATFORM_CPU_64_BIT
    static const Offset OFFSET_MAX =  (9223372036854775807L);
    static const Offset OFFSET_MIN = (-9223372036854775807L-1);
#else
    static const Offset OFFSET_MAX =  (9223372036854775807LL);
    static const Offset OFFSET_MIN = (-9223372036854775807LL-1);
#endif
#endif

    enum Whence {
        e_SEEK_FROM_BEGINNING = 0,
        e_SEEK_FROM_CURRENT   = 1,
        e_SEEK_FROM_END       = 2
    };

    enum {
        e_DEFAULT_GROW_BUFFER_SIZE = 65536
    };

    enum {
        e_ERROR_LOCKING_CONFLICT = 1
    };

    enum FileOpenPolicies {
        e_FILE_OPEN,
        e_FILE_CREATE,
        e_FILE_OPEN_OR_CREATE
    };

    enum FileIOPolicies {
        e_IO_READ_ONLY,
        e_IO_WRITE_ONLY,
        e_IO_APPEND_ONLY,
        e_IO_READ_WRITE,
        e_IO_READ_APPEND
    };

    enum FileTruncatePolicies {
        e_INIT_TRUNCATE,
        e_INIT_KEEP
    };

    // CLASS DATA
    static const FileDescriptor INVALID_FD;

    // CLASS METHODS
    static FileDescriptor open(
                      const char                *path,
                      enum FileOpenPolicies      openPolicy,
                      enum FileIOPolicies        ioPolicy,
                      enum FileTruncatePolicies  truncatePolicy = e_INIT_KEEP);
    static FileDescriptor open(
                      const bsl::string&         path,
                      enum FileOpenPolicies      openPolicy,
                      enum FileIOPolicies        ioPolicy,
                      enum FileTruncatePolicies  truncatePolicy = e_INIT_KEEP);
        // Open the file at the specified 'path', allowing writing if the
        // specified 'ioPolicy' is 'e_IO_READ_WRITE' or 'e_IO_WRITE_ONLY', and
        // allowing reading if 'ioPolicy' is 'e_IO_READ_WRITE' or
        // 'e_IO_READ_ONLY'.  If the specified 'openPolicy' is 'e_FILE_OPEN' or
        // 'e_FILE_OPEN_TRUNCATED', succeed only if the file exists.  If
        // 'openPolicy' is 'e_FILE_CREATE', succeed only if the file does not
        // exist, creating the file in the process.  If the 'openPolicy' is any
        // other value, open the file unconditionally, (creating the file if it
        // does not already exist).  If 'openPolicy' is 'e_FILE_OPEN_TRUNCATED'
        // or 'e_FILE_OPEN_TRUNCATED_OR_CREATE', the file will be of length 0
        // when a successful call to 'open' returns.  Optionally specify an
        // 'truncatePolicy' to indicate whether the file should allow writing
        // at any location ('e_INIT_KEEP') or whether the file should be opened
        // in append mode ('e_WRITE_APPEND').  The 'truncatePolicy' has no
        // effect if writing is not allowed.  Return a valid 'FileDesriptor'
        // for the file on success, or 'INVALID_FD' otherwise.  Note that when
        // a file is opened in 'append' mode, all writes will go to the end of
        // the file, even if there has been seeking on the file descriptor or
        // another process has changed the length of the file, though
        // append-mode writes are not guaranteed to be atomic.

    static int close(FileDescriptor descriptor);
        // Close the specified 'descriptor'.  Return 0 on success and a
        // non-zero value otherwise.

    static int getWorkingDirectory(bsl::string *path);
        // Load into the specified 'path' the absolute pathname of the current
        // working directory.  Return 0 on success and a non-zero value
        // otherwise.

    static int setWorkingDirectory(const bsl::string&  path);
    static int setWorkingDirectory(const char         *path);
        // Set the working directory of the current process to the specified
        // 'path'.  Return 0 on success and a non-zero value otherwise.

    static bool exists(const bsl::string&  path);
    static bool exists(const char         *path);
        // Return 'true' if there currently exists a file or directory at the
        // specified 'path', and 'false' otherwise.

    static bool isRegularFile(const bsl::string&  path,
                              bool                followLinks = false);
    static bool isRegularFile(const char         *path,
                              bool                followLinks = false);
        // Return 'true' if there currently exists a regular file at the
        // specified 'path', and 'false' otherwise.  If there is a symbolic
        // link at 'path', follow it only if the optionally-specified
        // 'followLinks' flag is 'true' (otherwise, return 'false' as the
        // symbolic link itself is not a regular file irrespective of the file
        // to which it points).  Platform-specific note: On POSIX, this is a
        // positive test on the "regular file" mode; on Windows, this is a
        // negative test on the "directory" attribute, i.e., on Windows,
        // everything that exists and is not a directory is a regular file.

    static bool isDirectory(const bsl::string&  path,
                            bool                followLinks = false);
    static bool isDirectory(const char         *path,
                            bool                followLinks = false);
        // Return 'true' if there currently exists a directory at the specified
        // 'path', and 'false' otherwise.  If there is a symbolic link at
        // 'path', follow it only if the optionally specified 'followLinks'
        // flag is 'true' (otherwise return 'false').  Platform-specific note:
        // On Windows, a "shortcut" is not a symbolic link.

    static int getLastModificationTime(bdet_Datetime      *time,
                                       const bsl::string&  path);
    static int getLastModificationTime(bdet_Datetime      *time,
                                       const char         *path);
        // Load into the specified 'time' the last modification time of the
        // file at the specified 'path', as reported by the filesystem.  Return
        // 0 on success, and a non-zero value otherwise.

    // TBD: write setModificationTime() when SetFileInformationByHandle()
    // becomes available on our standard Windows platforms.

    static int createDirectories(const bsl::string&  path,
                                 bool                leafIsDirectory = false);
    static int createDirectories(const char         *path,
                                 bool                leafIsDirectory = false);
        // Create any directories in 'path' which do not exist.  If the
        // optionally specified 'leafIsDirectory' is 'true', then treat the
        // last filename in the path as a directory and attempt to create it.
        // Otherwise, treat the last filename as a regular file and ignore it.
        // Return 0 on success, and a non-zero value if any needed directories
        // in the path could not be created.

    static void visitPaths(
                     const bsl::string&                               pattern,
                     const bdef_Function<void(*)(const char *path)>&  visitor);
    static void visitPaths(
                     const char                                      *pattern,
                     const bdef_Function<void(*)(const char *path)>&  visitor);
        // Call the specified 'visitor' functor for each path in the filesystem
        // matching the specified 'pattern'.  If 'visitor' deletes files or
        // directories during the search, the behavior is
        // implementation-dependent: 'visitor' may subsequently be called with
        // paths which have already been deleted, or it may not.  Note that
        // there is no stability risk in that case.  See 'findMatchingPaths'
        // for a discussion of how 'pattern' is interpreted.

    static void findMatchingPaths(bsl::vector<bsl::string> *result,
                                  const char               *pattern);
        // Load into the specified 'result' vector all paths in the filesystem
        // matching the specified 'pattern'.  The '*' character will match any
        // number of characters in a filename; however, this matching will not
        // span a directory separator (e.g., "logs/m*.txt" will not match
        // "logs/march/001.txt").  '?' will match any one character.  '*' and
        // '?' may be used any number of times in the pattern.  The special
        // directories "." and ".." will not be matched against any pattern.
        // Note that any initial contents of 'result' will be erased, and that
        // the paths in 'result' will not be in any particular guaranteed
        // order.
        //
        // WINDOWS-SPECIFIC NOTE: To support DOS idioms, the OS-provided search
        // function has behavior that we have chosen not to work around: an
        // extension consisting of wild-card characters ('?', '*') can match
        // an extension or *no* extension.  E.g., "file.?" matches "file.z",
        // but not "file.txt"; however, it also matches "file" (without any
        // extension).  Likewise, "*.*" matches any filename, including
        // filenames having no extension.

    static Offset getAvailableSpace(const bsl::string&  path);
    static Offset getAvailableSpace(const char         *path);
        // Return the number of bytes available for allocation in the file
        // system where the file or directory with the specified 'path'
        // resides, or a negative value if an error occurs.

    static Offset getAvailableSpace(FileDescriptor fd);
        // Return the number of bytes available for allocation in the file
        // system where the file with the specified descriptor 'fd' resides,
        // or a negative value if an error occurs.

    static Offset getFileSize(const bsl::string&  path);
    static Offset getFileSize(const char         *path);
        // Return the size, in bytes, of the file or directory at the specified
        // 'path', or a negative value if an error occurs.  Note that the size
        // of a symbolic link is the size of the file or directory to which it
        // points.

    static Offset getFileSizeLimit();
        // Return the file size limit for this process, 'OFFSET_MAX' if no
        // limit is set, or a negative value if an error occurs.  Note that
        // if you are doing any calculations involving the returned value, it
        // is recommended to check for 'OFFSET_MAX' specifically to avoid
        // integer overflow in your calculations.

    static int lock(FileDescriptor fd, bool lockWrite);
        // Acquire a lock for the file with the specified 'fd'.  If 'lockWrite'
        // is true, acquire an exclusive write lock; otherwise acquire a
        // (possibly) shared read lock.  The calling thread will block until
        // the lock is acquired.  Return 0 on success, and a non-zero value
        // otherwise.  Note that this operation locks the indicated file for
        // use by the current *process*, but the behavior is unspecified (and
        // platform-dependent) when either attempting to lock 'fd' multiple
        // times, or attempting to lock another descriptor referring to the
        // same file, within a single process.

    static int tryLock(FileDescriptor fd, bool lockWrite);
        // Acquire a lock for the file with the specified 'fd' if it is
        // currently available.  If 'lockWrite' is true, acquire an exclusive
        // write lock unless another process has any type of lock on the file.
        // If 'lockWrite' is false, acquire a shared read lock unless a process
        // has a write lock.  This method will not block.  Return 0 on success,
        // 'e_ERROR_LOCKING_CONFLICT' if the platform reports the lock could
        // not be acquired because another process holds a conflicting lock,
        // and a negative value for any other kind of error.  Note that this
        // operation locks the indicated file for the current *process*, but
        // the behavior is unspecified (and platform-dependent) when either
        // attempting to lock 'fd' multiple times, or attempting to lock
        // another descriptor referring to the same file, within a single
        // process.

    static int unlock(FileDescriptor fd);
        // Release any lock this process holds on the file with the specified
        // 'fd'.  Return 0 on success, and a non-zero value otherwise.

    static int map(FileDescriptor   fd,
                   void           **addr,
                   Offset           offset,
                   int              size,
                   int              mode);
        // Map the region of the specified 'size' bytes, starting at the
        // specified 'offset' bytes into the file with the specified 'fd'
        // descriptor to memory, and load into the specified 'addr' the address
        // of the mapped area.  Return 0 on success, and a non-zero value
        // otherwise.  The access permissions for mapping memory are defined by
        // the specified 'mode', which may be a combination of
        // 'bdesu_MemoryUtil::BDESU_ACCESS_READ',
        // 'bdesu_MemoryUtil::BDESU_ACCESS_WRITE' and
        // 'bdesu_MemoryUtil::BDESU_ACCESS_EXECUTE'.  Note that on failure, the
        // value of 'addr' is undefined.  Also note that mapping will succeed
        // even if there are fewer than 'offset + size' bytes in the specified
        // file, and an attempt to access the mapped memory beyond the end of
        // the file will result in undefined behavior (i.e., this function does
        // not grow the file to guarantee it can accommodate the mapped
        // region).

    static int unmap(void *addr, int size);
        // Unmap the memory mapping with the specified base address 'addr' and
        // specified 'size'.  Return 0 on success, and a non-zero value
        // otherwise.  The behavior is undefined unless the area with the
        // specified 'address' and 'size' was previously mapped with a 'map'
        // call.

    static int sync(char *addr, int numBytes, bool sync);
        // Synchronize the contents of the specified 'numBytes' of mapped
        // memory beginning at the specified 'addr' with the underlying file
        // on disk.  If the specified 'sync' flag is true, block until all
        // writes to nonvolatile media have actually completed; otherwise,
        // return once they have been scheduled.  Return 0 on success, and a
        // non-zero value otherwise.  The behavior is undefined unless 'addr'
        // is aligned on a page boundary, 'numBytes' is a multiple of
        // 'pageSize()', and '0 <= numBytes'.

    static Offset seek(FileDescriptor fd, Offset offset, int whence);
        // Set the file pointer associated with the specified 'fd' file
        // descriptor (used by calls to the 'read' and 'write' system calls)
        // according to the specified 'whence' behavior:
        //..
        //   * If 'whence' is e_SEEK_FROM_BEGINNING, set the pointer to
        //     'offset' bytes from the beginning of the file.
        //   * If 'whence' is e_SEEK_FROM_CURRENT, advance the pointer by
        //     'offset' bytes
        //   * If 'whence' is e_SEEK_FROM_END, set the pointer to 'offset'
        //     bytes beyond the end of the file.
        //..
        // Return the new location of the file pointer, in bytes from the
        // beginning of the file, on success; and -1 otherwise.  The effect on
        // the file pointer is undefined unless the file is on a device capable
        // of seeking.  Note that 'seek' does not change the size of the file
        // if the pointer advances beyond the end of the file; instead, the
        // next write at the pointer will increase the file size.

    static int read(FileDescriptor fd, void *buf, int numBytes);
        // Read 'numBytes' bytes beginning at the file pointer of the file with
        // the specified 'fd' into the specified 'buf' buffer.  Return
        // 'numBytes' on success; the number of bytes read if there were not
        // enough available; or a negative number on some other error.

    static int remove(const bsl::string&  path, bool recursive = false);
    static int remove(const char         *path, bool recursive = false);
        // Remove the file or directory at the specified 'path'.  If the 'path'
        // refers to a directory and the optionally specified 'recursive' flag
        // is 'true', recursively remove all files and directories within the
        // specified directory before removing the directory itself.  Return 0
        // on success and a non-zero value otherwise.  Note that if 'path' is a
        // directory, and the directory is not empty, and recursive is 'false',
        // this method will fail.  Also note that if the function fails when
        // 'recursive' is 'true', it may or may not have removed *some* files
        // or directories before failing.

    static int rollFileChain(const bsl::string& path, int maxSuffix);
    static int rollFileChain(const char        *path, int maxSuffix);
        // Remove the file at the specified 'path' appended with the specified
        // 'maxSuffix' using a '.' as a separator.  Then move the files with
        // the suffixes '.1' to '.maxSuffix-1' so they have new suffixes from
        // '.2' to '.maxSuffix'.  Finally, move 'path' to 'path' with a '.1'
        // suffix.  Return 0 on success, and non-zero otherwise.

    static int move(const bsl::string&  oldPath, const bsl::string&  newPath);
    static int move(const char         *oldPath, const char         *newPath);
        // Move the file or directory at the specified 'oldPath' to the
        // specified 'newPath'.  If there is a file or directory at 'newPath',
        // it will be removed and replaced.  In that case, 'newPath' must refer
        // to the same type of filesystem item as 'oldPath' - that is, they
        // must both be directories or both be files.  Return 0 on success, and
        // a non-zero value otherwise.

    static int write(FileDescriptor fd, const void *buf, int numBytes);
        // Write 'numBytes' beginning at the specified 'buf' address to the
        // file with the specified 'fd'.  Return 'numBytes' on success; the
        // number of bytes written if space was exhausted; or a negative value
        // on some other error.

    static int grow(FileDescriptor fd,
                    Offset         size,
                    bool           reserve = false,
                    bsl::size_t    bufferSize = e_DEFAULT_GROW_BUFFER_SIZE);
        // Grow the file with the specified 'fd' to the size of at least 'size'
        // bytes.  Return 0 on success, and a non-zero value otherwise.  If the
        // optionally specified 'reserve' flag is true, make sure the space on
        // disk is preallocated and not allocated on demand, preventing a
        // possible out-of-disk-space error when accessing the data on file
        // systems with sparse file support.  Preallocation is done by writing
        // unspecified data to file in blocks of the specified 'bufferSize'.
        // Note that if the size of the file is greater than or equal to
        // 'size', this function has no effect.  Also note that the contents of
        // the newly grown portion of the file is undefined.
};

// ============================================================================
//                          INLINE FUNCTION DEFINITIONS
// ============================================================================

                         // -------------------------
                         // struct bdesu_FileUtilUtf8
                         // -------------------------

// CLASS METHODS
inline
int bdesu_FileUtilUtf8::createDirectories(const bsl::string& path,
                                          bool               leafIsDirectory)
{
    return createDirectories(path.c_str(), leafIsDirectory);
}

inline
void bdesu_FileUtilUtf8::visitPaths(
                       const bsl::string&                              pattern,
                       const bdef_Function<void(*)(const char *path)>& visitor)
{
    return visitPaths(pattern.c_str(), visitor);
}

inline
bdesu_FileUtilUtf8::FileDescriptor
bdesu_FileUtilUtf8::open(const bsl::string&        path,
                         enum FileOpenPolicies     openPolicy,
                         enum FileIOPolicies       ioPolicy,
                         enum FileTruncatePolicies truncatePolicy)
{
    return open(path.c_str(), openPolicy, ioPolicy, truncatePolicy);
}

inline
bool bdesu_FileUtilUtf8::exists(const bsl::string& path)
{
    return exists(path.c_str());
}

inline
bool bdesu_FileUtilUtf8::isRegularFile(const bsl::string& path,
                                       bool               followLinks)
{
    return isRegularFile(path.c_str(), followLinks);
}

inline
bool bdesu_FileUtilUtf8::isDirectory(const bsl::string& path,
                                     bool               followLinks)
{
    return isDirectory(path.c_str(), followLinks);
}

inline
int bdesu_FileUtilUtf8::getLastModificationTime(bdet_Datetime      *time,
                                                const bsl::string&  path)
{
    BSLS_ASSERT_SAFE(time);

    return getLastModificationTime(time, path.c_str());
}

inline
int bdesu_FileUtilUtf8::remove(const bsl::string& path, bool recursive)
{
    return remove(path.c_str(), recursive);
}

inline
int bdesu_FileUtilUtf8::rollFileChain(const bsl::string& path, int maxSuffix)
{
    return rollFileChain(path.c_str(), maxSuffix);
}

inline
int bdesu_FileUtilUtf8::move(const bsl::string& oldPath,
                             const bsl::string& newPath)
{
    return move(oldPath.c_str(), newPath.c_str());
}

inline
int bdesu_FileUtilUtf8::setWorkingDirectory(const bsl::string& path)
{
    return setWorkingDirectory(path.c_str());
}

inline
bdesu_FileUtilUtf8::Offset bdesu_FileUtilUtf8::getFileSize(
                                                       const bsl::string& path)
{
    return getFileSize(path.c_str());
}

inline
bdesu_FileUtilUtf8::Offset bdesu_FileUtilUtf8::getAvailableSpace(
                                                       const bsl::string& path)
{
    return getAvailableSpace(path.c_str());
}

}  // close enterprise namespace

#endif

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2014
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ------------------------------ END-OF-FILE ---------------------------------
