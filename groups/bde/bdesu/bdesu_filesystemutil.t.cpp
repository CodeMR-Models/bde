// bdesu_filesystemutil.t.cpp                                         -*-C++-*-
#include <bdesu_filesystemutil.h>

#include <bdesu_memoryutil.h>
#include <bdesu_pathutil.h>

#include <bdede_charconvertutf16.h>
#include <bdef_bind.h>
#include <bdet_datetime.h>
#include <bdetu_systemtime.h>
#include <bsls_asserttest.h>
#include <bsls_platform.h>
#include <bsl_algorithm.h>
#include <bsl_c_errno.h>
#include <bsl_c_stdio.h>
#include <bsl_cstdlib.h>
#include <bsl_iostream.h>
#include <bsl_map.h>
#include <bsl_sstream.h>
#include <bsl_vector.h>

#ifndef BSLS_PLATFORM_OS_WINDOWS
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>

#else // BSLS_PLATFORM_OS_WINDOWS

#include <windows.h>  // for Sleep, GetLastError
#include <fcntl.h>    // for _O_U16TEXT
#include <io.h>
#endif

using namespace BloombergLP;
using namespace bsl;

//=============================================================================
//                                 TEST PLAN
//-----------------------------------------------------------------------------
// CLASS METHODS
// [ 2] FD open(const char *path, openPolicy, ioPolicy, truncatePolicy)
// [ 2] FD open(const string& path, openPolicy, ioPolicy, truncatePolicy)
// [ 3] void findMatchingPaths(bsl::vector<bsl::string> *,const char *)
// [ 4] bool isRegularFile(const bsl::string&, bool)
// [ 4] bool isRegularFile(const char *, bool)
// [ 4] bool isDirectory(const bsl::string&, bool)
// [ 4] bool isDirectory(const char *, bool)
// [ 5] int rollFileChain(const bsl::string&, int)
// [ 5] int rollFileChain(const char *, int)
// [ 6] Offset getAvailableSpace(const bsl::string&)
// [ 6] Offset getAvailableSpace(const char *)
// [ 6] Offset getAvailableSpace(FileDescriptor)
// [ 7] Offset getFileSize(const bsl::string&)
// [ 7] Offset getFileSize(const char *)
// [ 9] FD open(const char *p, bool writable, bool exist, bool append)
// [10] static Offset getFileSizeLimit()
// [11] int tryLock(FileDescriptor, bool ) (Unix)
// [12] int tryLock(FileDescriptor, bool ) (Windows)
// [14] int sync(char *, int , bool )
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [ 8] CONCERN: findMatchingPaths incorrect on ibm 64-bit
// [13] CONCERN: Open in append-mode behavior (particularly on windows)
// [15] CONCERN: Unix File Permissions for 'open'
// [16] CONCERN: Unix File Permissions for 'createDirectories'
// [17] CONCERN: UTF-8 Filename handling
// [18] USAGE EXAMPLE 1
// [19] USAGE EXAMPLE 2

//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
   if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
                    << J << "\t" \
                    << #K << ": " << K <<  "\n"; aSsErT(1, #X, __LINE__); } }

//=============================================================================
//                       SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", "<< flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number
#define T_()  cout << "\t" << flush;          // Print tab w/o newline

// ============================================================================
//                  NEGATIVE-TEST MACRO ABBREVIATIONS
// ----------------------------------------------------------------------------

#define ASSERT_SAFE_PASS(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_PASS(EXPR)
#define ASSERT_SAFE_FAIL(EXPR) BSLS_ASSERTTEST_ASSERT_SAFE_FAIL(EXPR)
#define ASSERT_PASS(EXPR)      BSLS_ASSERTTEST_ASSERT_PASS(EXPR)
#define ASSERT_FAIL(EXPR)      BSLS_ASSERTTEST_ASSERT_FAIL(EXPR)
#define ASSERT_OPT_PASS(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_PASS(EXPR)
#define ASSERT_OPT_FAIL(EXPR)  BSLS_ASSERTTEST_ASSERT_OPT_FAIL(EXPR)

//=============================================================================
//                           GLOBAL DATA FOR TESTING
//-----------------------------------------------------------------------------

#ifdef BSLS_PLATFORM_OS_WINDOWS
#   define PS "\\"
#else
#   define PS "/"
#endif

enum { NAME_ASCII, NAME_UTF8, NAME_ANSI };
static const char *const NAMES[] = {
    "name",                                      // ASCII
    "\x24\xc2\xa2\xe2\x82\xac\xf0\xa4\xad\xa2",  // utf-8
    "\xf1\xe5m\xea",                             // not utf-8
};
static const size_t NUM_NAMES  = sizeof NAMES / sizeof *NAMES;

#ifdef BSLS_PLATFORM_OS_WINDOWS
// 'NAME_ANSI' is not utf8, therefore the Windows implementation will refuse to
// create a file with that name.

static const size_t NUM_VALID_NAMES = NUM_NAMES - 1;
#else
static const size_t NUM_VALID_NAMES = NUM_NAMES;
#endif

//=============================================================================
//                  GLOBAL HELPER TYPE FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

typedef bdesu_FilesystemUtil Obj;

static int test;

#ifdef BSLS_PLATFORM_OS_WINDOWS
inline
bool isBackslash (char t)
{
    return t == '\\';
}

inline
bool isForwardSlash (char t)
{
    return t == '/';
}

#endif

void localTouch(const char *fileName)
{
    Obj::FileDescriptor fd = Obj::open(fileName,
                                       Obj::e_OPEN_OR_CREATE,
                                       Obj::e_READ_WRITE);
    ASSERT(Obj::k_INVALID_FD != fd);
    // ASSERT(1 == Obj::write(fd, "x", 1));

    Obj::close(fd);
}

void localSleep(int seconds)
{
#ifdef BSLS_PLATFORM_OS_UNIX
    sleep(seconds);
#else
    ::Sleep(seconds * 1000);
#endif
}

int localGetPId()
{
#ifdef BSLS_PLATFORM_OS_WINDOWS
    return static_cast<int>(GetCurrentProcessId());
#else
    return static_cast<int>(getpid());
#endif
}

static
bsl::string localGetcwd()
{
    bsl::string result;
    ASSERT(0 == Obj::getWorkingDirectory(&result));
    return result;
}

static
void localForkExec(bsl::string command)
{
#ifdef BSLS_PLATFORM_OS_UNIX
    if (0 == fork()) {
        // child process

        bsl::vector<char *>  argvec;
        const char          *endp = command.data() + command.length();

        for (char *pc = &command[0]; pc < endp; ++pc) {
            argvec.push_back(pc);
            while (*pc && ' ' != *pc) {
                ++pc;
            }
            *pc = 0;
        }
        argvec.push_back(0);

        execv(argvec[0], argvec.data());

        ASSERT(0 && "execv failed");
    }
#else
    STARTUPINFO sui;
    GetStartupInfo(&sui);

    PROCESS_INFORMATION proci;

    char *cmd = const_cast<char *>(command.c_str());
    bool  rc  = CreateProcess(NULL,     // lpApplicationName
                              cmd,      // lpCommandLine
                              NULL,     // lpProcessAttributes
                              NULL,     // lpThreadAttributes
                              true,     // bInheritHandles
                              0,        // dwCreationFlags
                              NULL,     // lpEnvironment
                              NULL,     // lpCurrentDirectory
                              &sui,     // lpStartupInfo - in
                              &proci);  // lpProcessInformation - out
    ASSERT(rc);
#endif
}

string rollupPaths(vector<bsl::string>& paths)
{
   string result;

   sort(paths.begin(), paths.end());

   for (vector<bsl::string>::const_iterator it = paths.begin();
        it != paths.end(); ++it) {
      result.append(*it);
      result.push_back(':');
   }

   if (!result.empty()) {
      result.erase(result.end()-1);
   }

#ifdef BSLS_PLATFORM_OS_WINDOWS
   replace_if(result.begin(), result.end(), isBackslash, '/');
#endif
   return result;
}

inline
void makeArbitraryFile(const char *path)
{
    Obj::FileDescriptor fd = Obj::open(path,
                                       Obj::e_OPEN_OR_CREATE,
                                       Obj::e_READ_WRITE);
    ASSERT(Obj::k_INVALID_FD != fd);
    ASSERT(5 == Obj::write(fd, "hello", 5));
    ASSERT(0 == Obj::close(fd));
}

bsl::string tempFileName(const char *fnTemplate = 0)
    // Return a temporary file name.  Optionally specify 'fnTemplate' to serve
    // as a part of the resulting name.  On Windows, optionally specify a non-
    // zero value for 'nocheck' to prevent the filename for being checked for
    // uniqueness (and, as a side-effect, from being created).
{
    bsl::string result;

#ifndef BSLS_PLATFORM_OS_WINDOWS
    bsl::ostringstream oss;
    oss << "tmp.fileutil." << test << '.' << localGetPId();
    if (fnTemplate) {
        oss << '.' << fnTemplate;
    }
    const bsl::string& fnTemplateStr = oss.str();
    fnTemplate = fnTemplateStr.c_str();

    result = fnTemplate + string("_XXXXXX");
    close(mkstemp(&result[0]));
#else
    // We can't make proper use of 'fnTemplate' on Windows.  We have created
    // a local directory to put our files in and chdir'ed to it, so
    // 'tmpPathBuf' should just be ".".  'GetTempFileName' is a really lame
    // utility, other than the path, it allows us to specify only 3 chars of
    // file name (!????!!!!!).
    //: o The first will be 'T' (for 'tmp').
    //: o The next will be 'A' + test case #, accomodating up to 25 test cases.
    //: o The third will be 'A' - 1 + '# of calls' allowing this function to
    //:   be called 26 times in any one process (each test case is run in a
    //:   separate process).

    (void) fnTemplate;

    static int calls = 0;
    char tplt[4] = { 'T', char('A' + test), char('A' + calls++), '\0' };
    ASSERT(tplt[1] <= 'Z');
    ASSERT(tplt[2] <= 'Z');

    char tmpPathBuf[MAX_PATH] = { "." }, tmpNameBuf[MAX_PATH];
    GetTempFileName(tmpPathBuf, tplt, 0, tmpNameBuf);
    result = tmpNameBuf;
#endif

    // Test Invariant:

    ASSERT(!result.empty());
    return result;
}

class MMIXRand {
    // Pseudo-Random number generator based on Donald Knuth's 'MMIX'

    static const bsls::Types::Uint64 A = 6364136223846793005ULL;
    static const bsls::Types::Uint64 C = 1442695040888963407ULL;

    // DATA
    bsls::Types::Uint64 d_reg;
    bsl::stringstream   d_ss;
    char                d_outBuffer[17];

  public:
    // CREATOR
    MMIXRand()
    : d_reg(0)
    {
        memset(d_outBuffer, 0, sizeof(d_outBuffer));
    }

    // MANIPULATORS
    void munge()
        // Iterate 'd_reg' through one cycle
    {
        d_reg = d_reg * A + C;
    }

    void reset()
        // Reset 'd_reg'
    {
        d_reg = 0;
    }

    const char *display()
        // Display the current state of d_reg in hex
    {
        d_ss.str("");
        memset(d_outBuffer, ' ', 16);

        d_ss << bsl::hex << d_reg;
        const bsl::string& str = d_ss.str();
        LOOP_ASSERT(str.length(), 16 >= str.length());
        char *writeTo = d_outBuffer + (16 - str.length());

        bsl::strcpy(writeTo, str.c_str());
        ASSERT(16 == bsl::strlen(d_outBuffer));

        return d_outBuffer;
    }
};


void NoOpAssertHandler(const char *, const char *, int)
{
}

//=============================================================================
//                             USAGE EXAMPLES
//-----------------------------------------------------------------------------
// Ensures that the following functions in usage example 2 compiles and runs on
// all platforms.
//-----------------------------------------------------------------------------

namespace UsageExample2 {

///Example 2: Using 'bdesu_FilesystemUtil::visitPaths'
///- - - - - - - - - - - - - - - - - - - - - - - - - -
// 'bdesu_FilesystemUtil::visitPaths' enables clients to define a functor to
// operate on file paths that match a specified pattern.  In this example, we
// create a function that can be used to filter out files that have a last
// modified time within a particular time frame.
//
// First we define our filtering function:
//..
    void getFilesWithinTimeframe(bsl::vector<bsl::string> *vector,
                                 const char               *item,
                                 const bdet_Datetime&      start,
                                 const bdet_Datetime&      end)
    {
        bdet_Datetime datetime;
        int ret = bdesu_FilesystemUtil::getLastModificationTime(&datetime,
                                                                 item);

        if (ret) {
            return;                                                   // RETURN
        }

        if (datetime < start || datetime > end) {
            return;                                                   // RETURN
        }

        vector->push_back(item);
    }
//..
// Then, with the help of 'bdesu_FilesystemUtil::visitPaths' and
// 'bdef_BindUtil::bind', we create a function for finding all file paths that
// match a specified pattern and have a last modified time within a specified
// start and end time (both specified as a 'bdet_Datetime'):
//..
    void findMatchingFilesInTimeframe(bsl::vector<bsl::string> *result,
                                      const char               *pattern,
                                      const bdet_Datetime&      start,
                                      const bdet_Datetime&      end)
    {
        result->clear();
        bdesu_FilesystemUtil::visitPaths(
                                  pattern,
                                  bdef_BindUtil::bind(&getFilesWithinTimeframe,
                                                      result,
                                                      bdef_PlaceHolders::_1,
                                                      start,
                                                      end));
    }
//..

}  // close namespace UsageExample2

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    test = argc > 1 ? bsl::atoi(argv[1]) : 0;
    int verbose = argc > 2;
    int veryVerbose = argc > 3;
    int veryVeryVerbose = argc > 4;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    // Make 'program' be a absolute path to the executable, for when we fork /
    // exec from different directories.

    const bsl::string program = PS[0] == *argv[0] || bsl::strstr(argv[0],":\\")
                              ? bsl::string(argv[0])
                              : localGetcwd() + PS + argv[0];
    LOOP_ASSERT(program, Obj::exists(program));
    if (veryVerbose) P(program);

    bsl::string mainRoot;
    {
        // Must not call 'tempFileName' here, because 'tempFileName' would
        // create a plain file with the result name, and the attempt to
        // create the directory would fail.

#ifdef BSLS_PLATFORM_OS_UNIX
        char host[80];
        ASSERT(0 ==::gethostname(host, sizeof(host)));
#else
        const char *host = "win";     // 'gethostname' is difficult on
                                      // Windows, and we usually aren't using
                                      // nfs there anyway.
#endif

        bsl::ostringstream oss;
        oss << "tmp.filesystemutil.case_" << test << '.' << host << '.' <<
                                                                 localGetPId();
        mainRoot = oss.str();
    }
    if (veryVerbose) P(mainRoot);

    if (Obj::exists(mainRoot)) {
        // Sometimes the cleanup at the end of this program is unable to clean
        // up files, so we might encounter leftovers from a previous run, but
        // these can usually be deleted if sufficient time has elapsed.  If
        // we're not able to clean it up now, old files may prevent the test
        // case we're running this time from working.

        LOOP_ASSERT(mainRoot, 0 == Obj::remove(mainRoot, true));
    }
    ASSERT(0 == Obj::createDirectories(mainRoot, true));
    ASSERT(0 == Obj::setWorkingDirectory(mainRoot));

    switch(test) { case 0:
      case 19: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE 2
        //
        // Concerns:
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Define the functions provided in usage example 2, then test them
        //   and make sure they function as intended, by:
        //
        //   1) Create a series of file in 1 second intervals, and record the
        //      time of creation.
        //   2) Call the 'findMatchingFilesInTimeframe' function on the list of
        //      files created with the timeframe specified as an interval
        //      in between all file creations.
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Usage Example 2"
                          << "\n=======================" << endl;

        // make sure there isn't an unfortunately named file in the way

        bdesu_FilesystemUtil::remove("bdesu_filesystemutil.temp.2", true);
#ifdef BSLS_PLATFORM_OS_WINDOWS
        bsl::string logPath =  "bdesu_filesystemutil.temp.temp.2\\logs2\\";
#else
        bsl::string logPath =  "bdesu_filesystemutil.temp.temp.2/logs2/";
#endif

        ASSERT(0 == bdesu_FilesystemUtil::createDirectories(logPath.c_str(),
                                                          true));
        const int TESTSIZE = 10;
        bdet_Datetime modTime[TESTSIZE];

        for (int i = 0; i < TESTSIZE; ++i) {
            stringstream s;
            s << logPath << "file" << i << ".log";

            if (veryVerbose) {
                cout << "Creating file: " << s.str() << endl;
            }

            bdesu_FilesystemUtil::FileDescriptor fd
                         = bdesu_FilesystemUtil::open(s.str(),
                                                      Obj::e_OPEN_OR_CREATE,
                                                      Obj::e_READ_WRITE);
            ASSERT(bdesu_FilesystemUtil::k_INVALID_FD != fd);

            const char buffer[] = "testing";
            int bytes           = sizeof buffer;

            bdesu_FilesystemUtil::write(fd, buffer, bytes);
            bdesu_FilesystemUtil::close(fd);

#ifdef BSLS_PLATFORM_OS_WINDOWS
            Sleep(1000);  // 'Sleep' is in milliseconds on Windows.
#else
            sleep(1);
#endif

            bdesu_FilesystemUtil::getLastModificationTime(&modTime[i],
                                                           s.str());
            if (veryVerbose) {
                cout << "\tLast modification time: " << modTime[i] << endl;
            }
        }

        bsl::vector<bsl::string> results;
        bsl::string pattern = logPath + "*.log";
        const int START = 3;
        const int END   = 6;
        UsageExample2::findMatchingFilesInTimeframe(&results,
                                                    pattern.c_str(),
                                                    modTime[START],
                                                    modTime[END]);

        if (veryVerbose) {
            cout << "List of files found: " << endl;
            for (int i = 0; i < (int) results.size(); ++i) {
                bsl::cout << "\t" << results[i] << endl;
            }
        }

        ASSERT(results.size() == END - START + 1);
        ASSERT(0 == bdesu_PathUtil::popLeaf(&logPath));
        ASSERT(0 == bdesu_FilesystemUtil::remove(logPath.c_str(), true));
      } break;
      case 18: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE 1
        //
        // Concerns:
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Run the usage example 1
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting Usage Example 1"
                          << "\n=======================" << endl;

        // make sure there isn't an unfortunately named file in the way

        bdesu_FilesystemUtil::remove("bdesu_filesystemutil.temp.temp.1");

///Example 1: General Usage
/// - - - - - - - - - - - -
// In this example, we start with a (relative) native path to a directory
// containing log files:
//..
    #ifdef BSLS_PLATFORM_OS_WINDOWS
      bsl::string logPath = "bdesu_filesystemutil.temp.temp.1\\logs";
    #else
      bsl::string logPath = "bdesu_filesystemutil.temp.temp.1/logs";
    #endif
//..
// Suppose that we want to separate files into "old" and "new" subdirectories
// on the basis of modification time.  We will provide paths representing these
// locations, and create the directories if they do not exist:
//..
    bsl::string oldPath(logPath), newPath(logPath);
    bdesu_PathUtil::appendRaw(&oldPath, "old");
    bdesu_PathUtil::appendRaw(&newPath, "new");
    int rc = bdesu_FilesystemUtil::createDirectories(oldPath.c_str(), true);
    ASSERT(0 == rc);
    rc = bdesu_FilesystemUtil::createDirectories(newPath.c_str(), true);
    ASSERT(0 == rc);
//..
// We know that all of our log files match the pattern "*.log", so let's search
// for all such files in the log directory:
//..
    bdesu_PathUtil::appendRaw(&logPath, "*.log");
    bsl::vector<bsl::string> logFiles;
    bdesu_FilesystemUtil::findMatchingPaths(&logFiles, logPath.c_str());
//..
// Now for each of these files, we will get the modification time.  Files that
// are older than 2 days will be moved to "old", and the rest will be moved to
// "new":
//..
    bdet_Datetime modTime;
    bsl::string   fileName;
    for (bsl::vector<bsl::string>::iterator it = logFiles.begin();
                                                  it != logFiles.end(); ++it) {
      ASSERT(0 == bdesu_FilesystemUtil::getLastModificationTime(&modTime,
                                                                *it));
      ASSERT(0 == bdesu_PathUtil::getLeaf(&fileName, *it));
      bsl::string *whichDirectory =
                  2 < (bdetu_SystemTime::nowAsDatetime() - modTime).totalDays()
                  ? &oldPath
                  : &newPath;
      bdesu_PathUtil::appendRaw(whichDirectory, fileName.c_str());
      ASSERT(0 == bdesu_FilesystemUtil::move(it->c_str(),
                                             whichDirectory->c_str()));
      bdesu_PathUtil::popLeaf(whichDirectory);
    }
//..

#if 0
        // file i/o

        // create a new file
        bdesu_FilesystemUtil::FileDescriptor fd = bdesu_FilesystemUtil::open(
                                                         "tempfile",.
                                                         Obj::e_OPEN_OR_CREATE,
                                                         Obj::e_READ_WRITE);
        ASSERT(fd != bdesu_FilesystemUtil::k_INVALID_FD);
        // allocate a buffer with the size equal to memory page size and
        // fill with some data
        int size = bdesu_FilesystemUtil::pageSize();
        char* buf = new char[size];
        for(int i=0; i<size; ++i) {
            buf[i] = i & 0xFF;
        }

        // write data to the file
        bdesu_FilesystemUtil::seek(fd, size,
                                         bdesu_FilesystemUtil::FROM_BEGINNING);
        int rc = bdesu_FilesystemUtil::write(fd, buf, size);
        ASSERT(rc == size);

        // map the data page into memory
        char* data;
        rc = bdesu_FilesystemUtil::map(fd, (void**)&data, 0, size, true);
        ASSERT(0 == rc);

        // verify the data is equal to what we have written
        ASSERT(0 != data);
        ASSERT(0 == memcmp(buf, data, size));

        // unmap the page, delete the buffer and close the file
        rc = bdesu_FilesystemUtil::unmap(data, size);
        ASSERT(0 == rc);
        delete[] buf;
        bdesu_FilesystemUtil::close(fd);
#endif

        // NOT IN USAGE EXAMPLE: CLEAN UP

        ASSERT(0 == bdesu_PathUtil::popLeaf(&logPath));
        ASSERT(0 == bdesu_PathUtil::popLeaf(&logPath));
        ASSERT(0 == bdesu_FilesystemUtil::remove(logPath.c_str(), true));
      } break;
      case 17: {
        // --------------------------------------------------------------------
        // TESTING: UTF-8 Filenames
        //
        // DRQS 45032518 requested that for Windows, we treat file names as
        // being UTF-8 encoded and translate them to wide-character format so
        // as to enable correct use of native-language names.  The change made
        // in response is to attempt such conversion, and fail if the
        // conversion does not succeed.
        //
        // Concerns:
        //: 1 We can convert from wchar_t to utf-8 filenames and then back
        //:   again, getting back the original wchar_t name.  Note that this
        //:   does not test 'bdesu_FilesystemUtil' functionality, but is
        //:   necessary for further testing.
        //:
        //: 2 We can create files using the utf-8 names.
        //:
        //: 3 We can retrieve these names using path searching.
        //:
        //: 4 In Windows, the names should be the wchar_t names.
        //
        // Plan:
        //: 1 Create a set of wchar_t filenames, convert them to utf-8 and back
        //:   again, and verify that the original names are recovered. (C-1)
        //:
        //: 2 Create each of the files using its utf-8 name, write to it, and
        //:   close it, checking for failures.  (C-2)
        //:
        //: 3 Use 'bdesu_FilesystemUtil::findMatchingPaths' to look up the
        //:   names we just created and verify that the returned names are the
        //:   full set of utf-8 names we created.  (C-3)
        //:
        //: 4 In Windows, use the wchar_t path lookup function to look up each
        //:   wchar_t name and verify that it is the correct name. (C-4)
        // --------------------------------------------------------------------

        if (verbose) cout <<
            "TESTING: UTF-8 Filenames\n"
            "========================\n";

        static const wchar_t *const filenames[] = {
            L"\u00e3\u00e5\u00e8\u00e9\u00ee\u00ef\u00f4\u00f5\u00f9\u00fa",
            L"a\u0303\u00e5\u00e8\u00e9\u00ee\u00ef\u00f4\u00f5\u00f9\u00fa",
            L"\u00e3a\u030a\u00e8\u00e9\u00ee\u00ef\u00f4\u00f5\u00f9\u00fa",
            L"\u00e3\u00e5e\u0300\u00e9\u00ee\u00ef\u00f4\u00f5\u00f9\u00fa",
            L"\u00e3\u00e5\u00e8e\u0301\u00ee\u00ef\u00f4\u00f5\u00f9\u00fa",
            L"\u00de\u0127\u2021\u20ac\u00b2\u2116",
            L"\u03b6\u0434\u05d8\u0679\u0564\u0e3f",
            L"\u00e3\u00e5\u00e8\u00e9i\u0302\u00ef\u00f4\u00f5\u00f9\u00fa",
            L"\u00e3\u00e5\u00e8\u00e9\u00eei\u0308\u00f4\u00f5\u00f9\u00fa",
            L"\u00e3\u00e5\u00e8\u00e9\u00ee\u00efo\u0302\u00f5\u00f9\u00fa",
            L"\u00e3\u00e5\u00e8\u00e9\u00ee\u00ef\u00f4o\u0303\u00f9\u00fa",
            L"\u00e3\u00e5\u00e8\u00e9\u00ee\u00ef\u00f4\u00f5u\u0300\u00fa",
            L"\u00e3\u00e5\u00e8\u00e9\u00ee\u00ef\u00f4\u00f5\u00f9u\u0301",
            L"a\u0303a\u030ae\u0300e\u0301i\u0302i\u0308o\u0302o\u0303u"
                                                            L"\u0300u\u0301",
        };
        static const size_t NUM_FILES = sizeof filenames / sizeof *filenames;

        // make sure there isn't an unfortunately named file in the way

        bsl::string dir = "bdesu_filesystemutil.temp.17";
        bsl::string logPath = dir;

        Obj::remove(dir, true);
        bdesu_PathUtil::appendRaw(&logPath, "logs2");

        ASSERT(0 == Obj::createDirectories(logPath.c_str(), true));

        for (size_t fi = 0; fi < NUM_FILES; ++fi) {
            const wchar_t *const NAME = filenames[fi];
            bsl::string          narrow;
            bsl::wstring         wide;
            int                  rc;

            // Verify that converting the filenames from wchar_t to utf-8 and
            // back again leaves them unchanged.

#ifdef BSLS_PLATFORM_OS_WINDOWS  // Posix 'wcout' is too chancy to try.
            if (veryVerbose) {
                int mode = _setmode(_fileno(stdout), _O_U16TEXT);

                wcout << L"\tfi = " << fi << L", NAME = " << NAME << endl;
                _setmode(_fileno(stdout), mode);
            }
#endif

            rc = bdede_CharConvertUtf16::utf16ToUtf8(&narrow, NAME);

            LOOP_ASSERT(rc, rc == 0);

            rc = bdede_CharConvertUtf16::utf8ToUtf16(&wide, narrow.c_str());

            LOOP_ASSERT(rc, rc == 0);
            ASSERT(bsl::wstring(NAME) == wide);

            bsl::string name = logPath;
            bdesu_PathUtil::appendRaw(&name, (narrow + ".log").c_str());

            if (veryVerbose) { T_() cout << "Creating file: "; P(name) }

            Obj::FileDescriptor fd = Obj::open(name,
                                               Obj::e_OPEN_OR_CREATE,
                                               Obj::e_READ_WRITE);

            LOOP_ASSERT(fd, Obj::k_INVALID_FD != fd);

            Obj::write(fd, name.c_str(), static_cast<int>(name.length()));
            Obj::close(fd);
        }

        for (size_t ni = 0; ni < NUM_NAMES; ++ni) {
            const char *const NAME = NAMES[ni];

            bsl::string name = logPath;
            bdesu_PathUtil::appendRaw(&name, NAME);
            name += ".log";

            if (veryVerbose) { T_() cout << "Creating file: "; P(name) }

            Obj::FileDescriptor fd = Obj::open(name,
                                               Obj::e_OPEN_OR_CREATE,
                                               Obj::e_READ_WRITE);

#ifdef BSLS_PLATFORM_OS_WINDOWS
            LOOP2_ASSERT(fd, ni,
                               (ni == NAME_ANSI) == (Obj::k_INVALID_FD == fd));
#else
            LOOP_ASSERT(fd, Obj::k_INVALID_FD != fd);
#endif

            if (Obj::k_INVALID_FD != fd) {
                Obj::write(fd, name.c_str(), static_cast<int>(name.length()));
                Obj::close(fd);
            }
        }

        bsl::vector<bsl::string> results;
        bsl::string              pattern = logPath;

        bdesu_PathUtil::appendRaw(&pattern, "*.log");
        Obj::findMatchingPaths(&results, pattern.c_str());

        LOOP_ASSERT(results.size(),
                                NUM_FILES + NUM_VALID_NAMES == results.size());

#ifdef BSLS_PLATFORM_OS_WINDOWS
        // Use the Windows 'wchar_t' interface to find the files, showing that
        // they have the correct 16-bit filenames.

        for (size_t i = 0; i < NUM_FILES; ++i) {
            WIN32_FIND_DATAW   findDataW;
            const bsl::wstring name = bsl::wstring(filenames[i]) + L".log";
            const bsl::wstring path =
                            L"bdesu_filesystemutil.temp.17\\logs2\\" + name;

            if (veryVerbose) {
                int mode = _setmode(_fileno(stdout), _O_U16TEXT);

                wcout << L"\ti = " << i << L", path = " << path << endl;
                _setmode(_fileno(stdout), mode);
            }

            const HANDLE handle = FindFirstFileW(path.c_str(), &findDataW);

            LOOP_ASSERT(handle, INVALID_HANDLE_VALUE != handle);
            ASSERT(bsl::wstring(findDataW.cFileName) == name);

            FindClose(handle);
        }

        // Use the Windows 'A' interface to find the files, showing that they
        // have the correct 8-bit filenames.

        for (size_t i = 0; i < NUM_NAMES; ++i) {
            WIN32_FIND_DATAA findDataA;
            const bsl::string name = bsl::string(NAMES[i]) + ".log";
            const bsl::string path =
                           "bdesu_filesystemutil.temp.17" PS "logs2" PS + name;

            if (veryVerbose) { T_() P_(i) P(name) }

            const HANDLE handle = FindFirstFileA(path.c_str(), &findDataA);

            // 'NAME_UTF8' will fail to match a file through the 'A' interface.
            // 'NAME_ANSI' will have failed on the 'open' call above, so the
            // file will not exist at all.

            LOOP2_ASSERT(handle, i,
                        (i == NAME_ASCII) == (INVALID_HANDLE_VALUE != handle));

            if (INVALID_HANDLE_VALUE != handle) {
                FindClose(handle);
            }
        }
#endif

        // Check that the filenames we get are the ones we made (but not
        // necessarily in the same order).

        typedef bsl::map<bsl::string, int> FileSet;
        FileSet fileset;

        for (size_t i = 0; i < NUM_FILES; ++i) {
            // Increment count for each file found.

            ++fileset[results[i]];

            if (veryVerbose) {
                T_() cout << "Found file "; P_(i) P(results[i])
            }

            bsl::string path = logPath;
            bsl::string name;

            bdede_CharConvertUtf16::utf16ToUtf8(&name, filenames[i]);
            bdesu_PathUtil::appendRaw(&path, (name + ".log").c_str());

            // Decrement count for each file expected.

            --fileset[path];
        }

        for (size_t i = NUM_FILES; i < results.size(); ++i) {
            // Increment count for each file found.

            ++fileset[results[i]];

            if (veryVerbose) {
                T_() cout << "Found file "; P_(i) P(results[i])
            }

            bsl::string path = logPath;
            bsl::string name = NAMES[i - NUM_FILES];

#ifdef BSLS_PLATFORM_OS_WINDOWS
            if (i == NUM_FILES + NAME_ANSI) {
                // This file name was not UTF-8, so we should have failed to
                // create it.

                continue;
            }
#endif

            bdesu_PathUtil::appendRaw(&path, (name + ".log").c_str());

            // Decrement count for each file expected.

            --fileset[path];
        }

        // Check that found and expected file sets are the same.

        for (FileSet::const_iterator itr = fileset.begin(),
                                     end = fileset.end();
             itr != end; ++itr) {

            if (veryVerbose) {
                T_() cout << "Checking file "; P(itr->first)
            }

            LOOP2_ASSERT(itr->first, itr->second, 0 == itr->second);
        }

        LOOP_ASSERT(dir, 0 == Obj::remove(dir, true));

        // Check narrow and wide lookups.

        for (size_t i = 0; i < NUM_NAMES; ++i) {
            const char *const NAME = NAMES[i];

            Obj::FileDescriptor fd = Obj::open(NAME,
                                               Obj::e_OPEN_OR_CREATE,
                                               Obj::e_READ_WRITE);

#ifdef BSLS_PLATFORM_OS_WINDOWS
            LOOP2_ASSERT(fd, i, (NAME_ANSI == i) == (Obj::k_INVALID_FD == fd));

            if (Obj::k_INVALID_FD != fd) {
                Obj::close(fd);
            }
#else
            LOOP3_ASSERT(i, NAME, fd, Obj::k_INVALID_FD != fd);

            Obj::close(fd);
#endif

            bsl::vector<bsl::string> results;

            if (veryVerbose) { T_() cout << "Finding "; P(NAME) }

            Obj::findMatchingPaths(&results, NAME);

#ifdef BSLS_PLATFORM_OS_WINDOWS
            if (NAME_ANSI == i) {
                LOOP_ASSERT(results.size(), 0 == results.size());

                ASSERT(0 != Obj::remove(NAME, false));
            }
            else {
                LOOP_ASSERT(results.size(), 1    == results.size());
                LOOP_ASSERT(results[0],     NAME == results[0]);

                LOOP_ASSERT(NAME, 0 == Obj::remove(NAME, false));
            }
#else
            LOOP_ASSERT(results.size(), 1    == results.size());
            LOOP_ASSERT(results[0],     NAME == results[0]);

            LOOP_ASSERT(NAME, 0 == Obj::remove(NAME, false));
#endif
        }
      } break;
      case 16: {
        // --------------------------------------------------------------------
        // TESTING: Unix File Permissions for 'createDirectories'
        //
        // Concerns:
        //: 1 The permissions of a file created with 'createDirectories' on
        //:   unix are chmod 0777.  Although not (currently) contractually
        //:   guaranteed, this matches the behavior for std::fstream and is
        //:   consistent with the use of a umask (see DRQS 40563234).
        //
        // Plan:
        //: 1 Create a directory
        //: 2 Read its permissions via 'stat64' or 'stat'.
        //: 3 Observe that the permission are chmod 0777 (C-1).
        // --------------------------------------------------------------------

        if (verbose) cout <<
            "TESTING: Unix File Permissions for 'createDirectories\n"
            "=====================================================\n";

#ifdef BSLS_PLATFORM_OS_WINDOWS
        if (verbose) cout << "TEST SKIPPED ON WINDOWS\n";
#else
        umask(0);

        if (verbose) cout << "Testing 'createDirectories'\n";
        {
            const bsl::string& testBaseDir = tempFileName(
                                         "tmp.bdesu_filesystemutil_16.mkdir1");
            bsl::string fullPath = testBaseDir;
            bdesu_PathUtil::appendRaw(&fullPath, "dir2");

            if (veryVerbose) { P(fullPath); }

            (void) Obj::remove(testBaseDir, true);

            int rc = Obj::createDirectories(fullPath, true);
            ASSERT(0 == rc);

            ASSERT(Obj::exists(testBaseDir));
            ASSERT(Obj::exists(fullPath));

# ifdef BSLS_PLATFORM_OS_CYGWIN
            struct stat info;
            ASSERT(0 == ::stat(  fullPath.c_str(), &info));
# else
            struct stat64 info;
            ASSERT(0 == ::stat64(fullPath.c_str(), &info));
# endif
            info.st_mode &= 0777;

            enum { EXPECTED_PERMS = S_IRUSR|S_IWUSR|S_IXUSR |
                                    S_IRGRP|S_IWGRP|S_IXGRP |
                                    S_IROTH|S_IWOTH|S_IXOTH };

            const bool eqLeafDir = EXPECTED_PERMS == info.st_mode;

            if (veryVeryVerbose || !eqLeafDir) {
                bsl::ios_base::fmtflags flags = cout.flags();
                cout << bsl::oct << "Leaf dir: ";
                P_(EXPECTED_PERMS);    P(info.st_mode);
                cout.flags(flags);
            }
            ASSERT(eqLeafDir);

# ifdef BSLS_PLATFORM_OS_CYGWIN
            ASSERT(0 == ::stat(  testBaseDir.c_str(), &info));
# else
            ASSERT(0 == ::stat64(testBaseDir.c_str(), &info));
# endif
            info.st_mode &= 0777;

            const bool eqBaseDir = EXPECTED_PERMS == info.st_mode;

            if (veryVeryVerbose || !eqBaseDir) {
                bsl::ios_base::fmtflags flags = cout.flags();
                cout << bsl::oct << "Base dir: ";
                P_(EXPECTED_PERMS);    P(info.st_mode);
                cout.flags(flags);
            }
            ASSERT(eqBaseDir);

            ASSERT(0 == Obj::remove(testBaseDir, true));
        }
#endif
      } break;
      case 15: {
        // --------------------------------------------------------------------
        // TESTING: Unix File Permissions for 'open'
        //
        // Concerns:
        //: 1 The permissions of a file created with 'open' on unix are chmod
        //:   0666.  Although not (currently) contractually guaranteed, this
        //:   matches the behavior for std::fstream and is consistent with the
        //:   use of a umask (see DRQS 40563234).
        //
        // Plan:
        //: 1 Open a file, write some data to it, and close it.
        //: 2 Read its permissions via 'stat64' or 'stat'.
        //: 3 Observe that the permission are chmod 0666 (C-1).
        // --------------------------------------------------------------------

        if (verbose) cout <<
            "TESTING: Unix File Permissions for 'open'\n"
            "=========================================\n";

#ifdef BSLS_PLATFORM_OS_WINDOWS
        if (verbose) cout << "TEST SKIPPED ON WINDOWS\n";
#else
        umask(0);

        if (verbose) cout << "Testing 'open'\n";
        {
            typedef Obj::FileDescriptor FD;

            const bsl::string& testFile = tempFileName(
                                       "tmp.bdesu_filesystemutil_15.open.txt");
            if (veryVerbose) P(testFile);

            (void) Obj::remove(testFile, false);

            FD fd = Obj::open(testFile,
                              Obj::e_OPEN_OR_CREATE,
                              Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fd);

            const char *str = "To be or not to be\n";
            const int len = static_cast<int>(bsl::strlen(str));
            ASSERT(len == Obj::write(fd, str, len));

            ASSERT(0 == Obj::close(fd));

# ifdef BSLS_PLATFORM_OS_CYGWIN
            struct stat info;
            ASSERT(0 == ::stat(  testFile.c_str(), &info));
# else
            struct stat64 info;
            ASSERT(0 == ::stat64(testFile.c_str(), &info));
# endif
            info.st_mode &= 0777;
            const bool eq =
                          (S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH|S_IWOTH)
                                                               == info.st_mode;
            if (veryVerbose || !eq) {
                bsl::ios_base::fmtflags flags = cout.flags();
                cout << bsl::oct;
                P_((S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH|S_IWOTH));
                P(info.st_mode);
                cout.flags(flags);
            }
            ASSERT(eq);

            ASSERT(0 == Obj::remove(testFile, false));
        }
#endif
      } break;
      case 14: {
        // --------------------------------------------------------------------
        // TESTING: sync
        //
        // Note that this is a white-box test that aims to verify the
        // underlying system call is called with the appropriate arguments (it
        // is not a test of the operating system behavior).
        //
        // Unfortunately, I been unable to find an effective test for
        // concerns  1, 2, and 3, since I've been unable to observe memory
        // pages *not* synchronized to disk.
        //
        // Concerns:
        //: 1 On success the mapped bytes are synchronized with their values
        //:   in the file.
        //:
        //: 2 That only the region of memory at the specified location
        //:   is synchronized.
        //:
        //: 3 That only the indicated number of bytes are synchronized.
        //:
        //: 4 That on failure an error status is returned.
        //:
        //: 5 QoI: Asserted precondition violations are detected when enabled.
        //
        //
        //:Plan:
        //: 1 Call 'sync' with valid arguments and verify it returns
        //:   successfully. (C-1..3)
        //:
        //: 2 Call 'sync' with an invalid set of arguments (having disabled
        //:   assertions that would prevent the arguments being supplied to the
        //:   underlying system call)  (C-4)
        //:
        //: 3 Verify that, in appropriate build modes, defensive checks are
        //:   triggered for argument values (using the 'BSLS_ASSERTTEST_*'
        //:   macros).  (C-5)
        //
        // Testing:
        //   int sync(char *, int , bool )
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "TESTING: 'sync'" << endl
                          << "===============" << endl;

        typedef Obj::FileDescriptor FD;

        // Note that there appear to be '#define' for PAGESIZE and PAGE_SIZE
        // on AIX.

        const int MYPAGESIZE = bdesu_MemoryUtil::pageSize();
        const int SIZE       = MYPAGESIZE;
        const int READ       = bdesu_MemoryUtil::BDESU_ACCESS_READ;
        const int READ_WRITE = bdesu_MemoryUtil::BDESU_ACCESS_READ |
                               bdesu_MemoryUtil::BDESU_ACCESS_WRITE;
        int         rc     = 0;
        Obj::Offset offset = 0;

        bsl::string testFileName(tempFileName());
        Obj::remove(testFileName);
        FD writeFd = Obj::open(testFileName,
                               Obj::e_OPEN_OR_CREATE,
                               Obj::e_READ_WRITE);
        FD readFd  = Obj::open(testFileName,
                               Obj::e_OPEN,
                               Obj::e_READ_ONLY);

        ASSERT(Obj::k_INVALID_FD != writeFd);
        ASSERT(Obj::k_INVALID_FD != readFd);

        offset = Obj::seek(writeFd, SIZE, Obj::e_SEEK_FROM_BEGINNING);
        ASSERT(SIZE == offset);
        rc = Obj::write(writeFd, testFileName.c_str(), 1);
        ASSERT(1 == rc);

        offset = Obj::seek(writeFd, 0, Obj::e_SEEK_FROM_BEGINNING);
        ASSERT(0 == offset);

        void *writeMemory, *readMemory;

        rc = Obj::map(writeFd, &writeMemory, 0, SIZE, READ_WRITE);
        ASSERT(0 == rc);

        rc = Obj::map(readFd,   &readMemory, 0, SIZE, READ);
        ASSERT(0 == rc);

        ASSERT(readFd != writeFd);

        char *writeBuffer = static_cast<char *>(writeMemory);

        {

            if (veryVerbose) {
                cout << "\tTesting msync is performed" << endl;
            }

            rc = Obj::sync(writeBuffer, SIZE, true);
            ASSERT(0 == rc);

            // I have not been able to fashion an effective test for 'sync'
            // because I've been unable to observe unsynchronized memory
            // mapped pages (so it cannot be determined whether 'sync' is
            // actually performing synchronization).  For reference, you can
            // find some experiments writing to mapped-memory, and read from a
            // different file descriptor to the same file, in
            // 'devgit:bde/bde-core' commit:
            //..
            //  commit a93a90d9c567d7a24994811f79c65b38c2cb9791
            //  Author: (Henry) Mike Verschell <hverschell@bloomberg.net>
            //  Date:   Fri Apr 19 16:28:50 2013 -0400
            //..
        }
        {
            if (veryVerbose) {
                cout << "\tTesting msync returns an error status" << endl;
            }

            // Note that, experimentally, the only sane way to force an error
            // code from sync is to pass a address that is not aligned on a
            // page boundary.  We must first disable our own assertion handler
            // in order for the underlying system call to be invoked.

            bsls::AssertFailureHandlerGuard hg(NoOpAssertHandler);

            int address;

            rc = Obj::sync((char *)&address, MYPAGESIZE, true);
            ASSERT(0 != rc);
#ifdef BSLS_PLATFORM_OS_UNIX
            // Note that this is a white-box test that we return 'errno' on
            // error, which is not required by the method contract.
            ASSERT(EINVAL == rc);
            if (veryVeryVerbose) {
                P(rc);
            }
#endif
        }
        {
            bsls::AssertFailureHandlerGuard hG(
                                            bsls::AssertTest::failTestDriver);
            if (veryVerbose) cout << "\tTest assertions." << endl;

            ASSERT_PASS(Obj::sync(writeBuffer, SIZE, true));
            ASSERT_FAIL(Obj::sync(0, SIZE, true));
            ASSERT_FAIL(Obj::sync(writeBuffer, SIZE / 2, true));
            ASSERT_FAIL(Obj::sync(writeBuffer + 1, SIZE, true));

        }
        Obj::close(writeFd);
        Obj::close(readFd);
        Obj::remove(testFileName);
      } break;
      case 13: {
        // --------------------------------------------------------------------
        // Append test
        //
        // Concerns:
        //   On Unix, if a file is opened for append, all writes to that file,
        //   event following an lseek or a write from another process, are to
        //   append to the end of the file.  The MSDN doc specifically says
        //   that the doc's author doesn't know what will happen, so this test
        //   is to verify whatever the behavior is.
        // --------------------------------------------------------------------

        typedef Obj::FileDescriptor FD;

        const char *testFile = "tmp.bdesu_filesystemutil_13.append.txt";
        const char *tag1     = "tmp.bdesu_filesystemutil_13.tag.1.txt";
        const char *success  = "tmp.bdesu_filesystemutil_13.success.txt";

        const char testString[] = { "123456789" };

        enum { SZ10 = sizeof(testString) };  // satisfy pedants who won't
        ASSERT(10 == SZ10);                  // tolerate a magic number

        Obj::Offset fs;                      // file size cache variable

        bool isParent = !verbose || bsl::string(argv[2]) != "child";
        if (isParent)  {
            if (verbose) cout << "APPEND TEST\n"
                                 "===========\n";

            Obj::remove(testFile);
            Obj::remove(tag1);
            Obj::remove(success);

            // First, test with lseek on one file desc

            FD fd = Obj::open(testFile,
                              Obj::e_OPEN_OR_CREATE,
                              Obj::e_READ_APPEND);      // append mode;
            ASSERT(Obj::k_INVALID_FD != fd);

            int rc = Obj::write(fd, testString, SZ10);
            ASSERT(SZ10 == rc);

            Obj::Offset off =Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
            ASSERT(0 == off);

            rc = Obj::write(fd, testString, SZ10);
            ASSERT(SZ10 == rc);

            LOOP_ASSERT(fs, 2 * SZ10 == (fs = Obj::getFileSize(testFile)));
            off = Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT);
            ASSERT(2 * SZ10 == off);

            // Next, see what happens when another file desc from the same
            // process writes to the end of the file.

            FD fd2 = Obj::open(testFile,
                               Obj::e_OPEN,
                               Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fd2);

            off = Obj::seek(fd2, 0, Obj::e_SEEK_FROM_END);
            LOOP_ASSERT(off, 2 * SZ10 == off);

            rc = Obj::write(fd2, testString, SZ10);
            ASSERT(SZ10 == rc);
            LOOP_ASSERT(fs, 3 * SZ10 == (fs = Obj::getFileSize(testFile)));

            rc = Obj::write(fd, testString, SZ10);
            ASSERT(SZ10 == rc);

            LOOP_ASSERT(fs, 4 * SZ10 == (fs = Obj::getFileSize(testFile)));
            off = Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT);
            LOOP_ASSERT(off, 4 * SZ10 == off);

            bsl::stringstream cmd;
            cmd << argv[0] << ' ' << argv[1] << " child";
            cmd << (verbose     ? " v" : "");
            cmd << (veryVerbose ? " v" : "");

            localForkExec(cmd.str().c_str());

            while (!Obj::exists(tag1)) {
                if (veryVerbose) Q(Parent sleeping);
                localSleep(1);
            }
            if (verbose) Q(Parent detected tag1);

            rc = Obj::write(fd, testString, SZ10);
            ASSERT(SZ10 == rc);

            LOOP_ASSERT(fs, 6 * SZ10 == (fs = Obj::getFileSize(testFile)));
            off = Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT);
            LOOP_ASSERT(off, 6 * SZ10 == off);

            ASSERT(Obj::exists(success));

            Obj::close(fd);
            Obj::close(fd2);
            Obj::remove(testFile);
            Obj::remove(tag1);
            Obj::remove(success);

            if (verbose) Q(Parent finished);

#ifdef BSLS_PLATFORM_OS_UNIX
            wait(0);
#endif
        }
        else {
            // child process

            verbose = veryVerbose;
            veryVerbose = veryVeryVerbose;
            veryVeryVerbose = false;

            ASSERT(0 == Obj::setWorkingDirectory(".."));

            ASSERT(Obj::exists(testFile));
            LOOP_ASSERT(fs, 4 * SZ10 == (fs = Obj::getFileSize(testFile)));

            FD fdChild = Obj::open(testFile,
                                   Obj::e_OPEN,
                                   Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fdChild);

            Obj::Offset off = Obj::seek(fdChild, 0, Obj::e_SEEK_FROM_END);
            LOOP_ASSERT(off, 4 * SZ10 == off);

            int rc = Obj::write(fdChild, testString, SZ10);
            ASSERT(SZ10 == rc);

            if (0 == testStatus) {
                localTouch(success);
            }

            localTouch(tag1);

            Obj::close(fdChild);

            if (verbose) Q(Child finished);

            // Exit main to avoid doing cleanup at end of 'main' twice.

            return testStatus;                                        // RETURN
        }
      } break;

      // ----------------------------------------------------------------------
      // TRYLOCK TESTS
      //
      // Concerns:
      //   That 'tryLock' returns proper status on failure, depending upon
      //   the type of failure.
      //
      //   Unix and Windows have different nuances about locking.  On Windows,
      //   any file that's opened for write by one process cannot be opened for
      //   read or write by any other process.  On Unix it can.  On Unix you
      //   cannot lock for write a file that is not opened for write.  On
      //   Windows you can.  On Windows if another process has a a file opened
      //   for read and locked for write, you can open it for read but an
      //   actual read from it will fail.
      //
      //   Initially the two were developed as one test.  At first I did it
      //   with fork and it worked on Unix, but it turns out fork doesn't exist
      //   on Windows, and I don't know what the corresponding windows call
      //   would be (searching for 'fork' on MSDN just refers you to
      //   stackOverflow talking about Unix fork).  So the tests were redone
      //   using 'system' rather than fork.  Later, when I did the 'append'
      //   test, I found that on Windows, 'system' was ignoring the '&' and
      //   blocking until the child process terminated.  This was not a problem
      //   in the tryLock tests, but it caused the append test to hang.  So
      //   'localForkExec' was written.
      //
      // Plan:
      //   Given that the behaviors are so different, I'm doing completely
      //   different tests for Unix and Windows.  Test 11 will test Unix, test
      //   12 will do Windows.
      // ----------------------------------------------------------------------

      case 12: {
        // --------------------------------------------------------------------
        // WINDOWS TRYLOCK TEST
        // --------------------------------------------------------------------

#ifdef BSLS_PLATFORM_OS_WINDOWS
        typedef Obj::FileDescriptor FD;

        int rc;

        bsl::string fileNameWrite   = "tmp.filesystemutil.case_12.write";
        bsl::string fileNameRead    = "tmp.filesystemutil.case_12.read";
        bsl::string fileNameSuccess = "tmp.filesystemutil.case_12.success";

        if (veryVerbose) {
            T_() P(fileNameWrite) T_() P(fileNameRead) T_() P(fileNameSuccess)
        }

        FD fdWrite = Obj::k_INVALID_FD;
        FD fdRead  = Obj::k_INVALID_FD;

        bool isParent = !verbose || bsl::string(argv[2]) != "child";
        if (isParent) {
            if (verbose) cout << "tryLock test\n"
                                 "============\n";
            if (verbose) {
                cout << "Parent:";
                for (int i = 0; i < argc; ++i) {
                    cout << ' ' << argv[i];
                }
                cout << endl;
            }

            // Not checking status, since these may not exist yet.

            (void) Obj::remove(fileNameWrite);
            (void) Obj::remove(fileNameRead);
            (void) Obj::remove(fileNameSuccess);

            // on Windows, a file opened for write is implicitly locked, but
            // a file opened for read may be locked for write.  So open
            // 'fdWrite' for read and lock it for write.

            fdWrite = Obj::open(fileNameWrite,
                                Obj::e_OPEN_OR_CREATE,
                                Obj::e_READ_WRITE);
            LOOP_ASSERT(fdWrite, Obj::k_INVALID_FD != fdWrite);
            rc = Obj::write(fdWrite, "woof", 4);
            LOOP_ASSERT(rc, 4 == rc);
#if 0
            rc = Obj::close(fdWrite);
            LOOP_ASSERT(rc, 0 == rc);
            fdWrite = Obj::open(fileNameWrite,
                                Obj::e_OPEN,
                                Obj::e_READ_ONLY);
            LOOP_ASSERT(fdWrite, Obj::k_INVALID_FD != fdWrite);
#endif

            fdRead  = Obj::open(fileNameRead,
                                Obj::e_OPEN_OR_CREATE,
                                Obj::e_READ_WRITE);
            LOOP_ASSERT(fdRead, Obj::k_INVALID_FD != fdRead);
            rc = Obj::write(fdRead , "woof", 4);
            LOOP_ASSERT(rc, 4 == rc);
#if 0
            rc = Obj::close(fdRead);
            LOOP_ASSERT(rc, 0 == rc);
            fdRead = Obj::open(fileNameRead,
                               Obj::e_OPEN,
                               Obj::e_READ_ONLY);
            LOOP_ASSERT(fdRead, Obj::k_INVALID_FD != fdRead);
#endif
            rc = Obj::tryLock(fdWrite, true);
            LOOP_ASSERT(rc, 0 == rc);
            rc = Obj::tryLock(fdRead,  false);
            LOOP_ASSERT(rc, 0 == rc);

            bsl::stringstream cmd;
            cmd << argv[0] << ' ' << argv[1] << " child";
            cmd << (verbose     ? " v" : "");
            cmd << (veryVerbose ? " v" : "");

            localForkExec(cmd.str().c_str());

            localSleep(3);

            LOOP_ASSERT(fileNameSuccess, Obj::exists(fileNameSuccess));

            rc = Obj::unlock(fdWrite);
            LOOP_ASSERT(rc, 0 == rc);
            rc = Obj::unlock(fdRead);
            LOOP_ASSERT(rc, 0 == rc);
            Obj::close(fdWrite);
            Obj::close(fdRead);

            if (verbose) Q(Locking closed file descriptor);

            rc = Obj::tryLock(fdWrite, false);
            LOOP_ASSERT(rc, 0 != rc);

            if (verbose) P(GetLastError());

            LOOP_ASSERT(GetLastError(), ERROR_INVALID_HANDLE==GetLastError());

            if (verbose) Q(Locking invalid file descriptor);

            rc = Obj::tryLock(Obj::k_INVALID_FD, false);
            LOOP_ASSERT(rc, 0 != rc);

            if (verbose) P(GetLastError());

            LOOP_ASSERT(GetLastError(), ERROR_INVALID_HANDLE==GetLastError());

            Obj::remove(fileNameWrite);
            Obj::remove(fileNameRead);
            Obj::remove(fileNameSuccess);
        }
        else {
            // child process

            if (veryVerbose) {
                cout << "Child:";
                for (int i = 0; i < argc; ++i) {
                    cout << ' ' << argv[i];
                }
                cout << endl;
            }

            verbose = veryVerbose;
            veryVerbose = veryVeryVerbose;
            veryVeryVerbose = false;

            ASSERT(0 == Obj::setWorkingDirectory(".."));

            LOOP_ASSERT(fileNameWrite, Obj::exists(fileNameWrite));
            LOOP_ASSERT(fileNameRead, Obj::exists(fileNameRead));

            char buf[5];
            const bsl::string WOOF = "woof";

            // A curious thing about Windows -- if a file was already open for
            // read and locked for write, you can open it for read, but you
            // can't actually read from it.

            fdWrite = Obj::open(fileNameWrite,
                                Obj::e_OPEN,
                                Obj::e_READ_WRITE);
            LOOP_ASSERT(fdWrite, Obj::k_INVALID_FD != fdWrite);

            fdRead  = Obj::open(fileNameRead,
                                Obj::e_OPEN,
                                Obj::e_READ_ONLY);
            LOOP_ASSERT(fdRead, Obj::k_INVALID_FD != fdRead);
            bsl::memset(buf, 0, sizeof(buf));
            rc = Obj::read(fdRead,  buf, 4);
            LOOP_ASSERT(rc, 4 == rc);
            LOOP_ASSERT(buf, WOOF == buf);

            if (verbose) Q(Locked for write twice);
            SetLastError(0);
            rc = Obj::tryLock(fdWrite, true);
            LOOP_ASSERT(rc, 0 != rc);
            LOOP_ASSERT(rc, Obj::k_ERROR_LOCKING_CONFLICT == rc);
            if (verbose) P(GetLastError());
            LOOP_ASSERT(GetLastError(), ERROR_LOCK_VIOLATION==GetLastError());

            if (verbose) Q(Locked for write then read);
            SetLastError(0);
            rc = Obj::tryLock(fdWrite, false);
            LOOP_ASSERT(rc, 0 != rc);
            LOOP_ASSERT(rc, Obj::k_ERROR_LOCKING_CONFLICT == rc);
            if (verbose) P(GetLastError());
            LOOP_ASSERT(GetLastError(), ERROR_LOCK_VIOLATION==GetLastError());

            if (verbose) Q(Locked for read then write);
            SetLastError(0);
            rc = Obj::tryLock(fdRead, true);
            LOOP_ASSERT(rc, 0 != rc);
            LOOP_ASSERT(rc, Obj::k_ERROR_LOCKING_CONFLICT == rc);
            if (verbose) P(GetLastError());
            LOOP_ASSERT(GetLastError(), ERROR_LOCK_VIOLATION==GetLastError());

            if (verbose) Q(Locked for read then read (succeeds));
            SetLastError(0);
            rc = Obj::tryLock(fdRead, false);
            LOOP_ASSERT(rc, 0 == rc);
            if (verbose) P(GetLastError());
            LOOP_ASSERT(GetLastError(), 0 == GetLastError());
            rc = Obj::unlock(fdRead);
            LOOP_ASSERT(rc, 0 == rc);

            rc = Obj::close(fdWrite);
            LOOP_ASSERT(rc, 0 == rc);
            rc = Obj::close(fdRead);
            LOOP_ASSERT(rc, 0 == rc);

            if (0 == testStatus) {
                // Touch the 'success' file to tell the parent process we
                // succeeded.

                FD fdSuccess = Obj::open(fileNameSuccess,
                                         Obj::e_OPEN_OR_CREATE,
                                         Obj::e_READ_WRITE);
                Obj::close(fdSuccess);
            }

            // Exit 'main' to avoid doing the cleanup at the end of 'main'
            // twice.

            return testStatus;                                        // RETURN
        }
#endif
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // UNIX TRYLOCK TEST
        //
        // Concerns:
        //   That 'tryLock' returns proper status on failure, depending upon
        //   the type of failure.
        //
        // Plan:
        //   This plan takes advantage of the fact that there is no implicit
        //   exclusionary nature to opening a file for write on Unix, and
        //   respects the fact that one cannot lock a file for write unless
        //   the fd is opened for write.
        //   Note that attempting to unlock a fd that was not successfully
        //   does not return an error status on Unix.
        // --------------------------------------------------------------------

#if defined(BSLS_PLATFORM_OS_UNIX) && !defined(BSLS_PLATFORM_OS_CYGWIN)
        // 'tryLock' appears to ALWAYS succeed on Cygwin.

        typedef Obj::FileDescriptor FD;
        enum { BEGINNING = Obj::e_SEEK_FROM_BEGINNING };

        int rc;

        // It is important not to use 'tempFileName' here because otherwise
        // the parent and child will have different file names.

        bsl::string fileNameWrite   = "tmp.filesystemutil.case_11.write.txt";
        bsl::string fileNameRead    = "tmp.filesystemutil.case_11.read.txt";
        bsl::string fileNameSuccess = "tmp.filesystemutil.case_11.success.txt";

        if (veryVerbose) {
            P_(fileNameWrite);    P_(fileNameRead);    P(fileNameSuccess);
        }

        FD fdWrite = Obj::k_INVALID_FD;
        FD fdRead  = Obj::k_INVALID_FD;

        bool isParent = !verbose || bsl::string(argv[2]) != "child";
        if (isParent) {
            if (verbose) cout << "tryLock test\n"
                                 "============\n";

            Obj::remove(fileNameWrite);
            Obj::remove(fileNameRead);
            Obj::remove(fileNameSuccess);

            fdWrite = Obj::open(fileNameWrite,
                                Obj::e_OPEN_OR_CREATE,
                                Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fdWrite);
            rc = Obj::write(fdWrite, "woof", 4);
            ASSERT(4 == rc);

            fdRead  = Obj::open(fileNameRead,
                                Obj::e_OPEN_OR_CREATE,
                                Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fdRead);
            rc = Obj::write(fdRead , "woof", 4);
            ASSERT(4 == rc);
            rc = Obj::close(fdRead);
            ASSERT(0 == rc);
            fdRead  = Obj::open(fileNameRead,
                                Obj::e_OPEN,
                                Obj::e_READ_ONLY);
            ASSERT(Obj::k_INVALID_FD != fdRead);

            // Unix can only lock a writable file for write

            rc = Obj::tryLock(fdRead,  true);
            ASSERT(0 != rc);
            ASSERT(EBADF == errno);

            rc = Obj::tryLock(fdWrite, true);
            ASSERT(0 == rc);
            rc = Obj::tryLock(fdRead,  false);
            ASSERT(0 == rc);

            bsl::stringstream cmd;
            cmd << argv[0] << ' ' << argv[1] << " child";
            cmd << (verbose     ? " v" : "");
            cmd << (veryVerbose ? " v" : "");

            localForkExec(cmd.str().c_str());

            localSleep(3);

            ASSERT(Obj::exists(fileNameSuccess));

            rc = Obj::unlock(fdWrite);
            ASSERT(0 == rc);
            rc = Obj::unlock(fdRead);
            ASSERT(0 == rc);
            Obj::close(fdWrite);
            Obj::close(fdRead);

            // try to lock a closed file descriptor

            if (verbose) Q(Locking closed file descriptor);
            errno = 0;
            rc = Obj::tryLock(fdWrite, false);
            ASSERT(0 != rc);
            if (verbose) P(errno);
            LOOP_ASSERT(errno, EBADF == errno);

            if (verbose) Q(Invalid file descriptor);
            errno = 0;
            rc = Obj::tryLock(Obj::k_INVALID_FD, false);
            ASSERT(0 != rc);
            if (verbose) P(errno);
            LOOP_ASSERT(errno, EBADF == errno);

            Obj::remove(fileNameWrite);
            Obj::remove(fileNameRead);
            Obj::remove(fileNameSuccess);
        }
        else {
            // child process

            verbose = veryVerbose;
            veryVerbose = veryVeryVerbose;
            veryVeryVerbose = false;

            ASSERT(0 == Obj::setWorkingDirectory(".."));

            ASSERT(Obj::exists(fileNameWrite));
            ASSERT(Obj::exists(fileNameRead));

            char buf[5];
            const bsl::string WOOF = "woof";

            fdWrite = Obj::open(fileNameWrite,
                                Obj::e_OPEN,
                                Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fdWrite);
            bsl::memset(buf, 0, sizeof(buf));
            rc = Obj::read(fdWrite, buf, 4);
            ASSERT(4 == rc);
            ASSERT(WOOF == buf);

            // Note on Unix we open fdRead for 'write' so we can try to lock
            // it for write later

            fdRead  = Obj::open(fileNameRead,
                                Obj::e_OPEN,
                                Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fdRead);
            bsl::memset(buf, 0, sizeof(buf));
            rc = Obj::read(fdRead,  buf, 4);
            ASSERT(4 == rc);
            ASSERT(WOOF == buf);

#if defined(BSLS_PLATFORM_OS_AIX)
            enum { COLLIDE = EACCES };
#else
            enum { COLLIDE = EAGAIN };
#endif
            if (verbose) Q(Locked for write twice);
            errno = 0;
            rc = Obj::tryLock(fdWrite, true);
            ASSERT(0 != rc);
            ASSERT(Obj::k_ERROR_LOCKING_CONFLICT == rc);
            if (verbose) P(errno);
            LOOP_ASSERT(errno, COLLIDE == errno);

            if (verbose) Q(Locked for write then read);
            errno = 0;
            rc = Obj::tryLock(fdWrite, false);
            ASSERT(0 != rc);
            ASSERT(Obj::k_ERROR_LOCKING_CONFLICT == rc);
            if (verbose) P(errno);
            LOOP_ASSERT(errno, COLLIDE == errno);

            if (verbose) Q(Locked for read then write);
            errno = 0;
            rc = Obj::tryLock(fdRead, true);
            ASSERT(0 != rc);
            ASSERT(Obj::k_ERROR_LOCKING_CONFLICT == rc);
            if (verbose) P(errno);
            LOOP_ASSERT(errno, COLLIDE == errno);

            if (verbose) Q(Locked for read then read);
            errno = 0;
            rc = Obj::tryLock(fdRead, false);
            ASSERT(0 == rc);
            if (verbose) P(errno);
            LOOP_ASSERT(errno, 0 == errno);
            rc = Obj::unlock(fdRead);
            ASSERT(0 == rc);

            rc = Obj::close(fdWrite);
            ASSERT(0 == rc);
            rc = Obj::close(fdRead);
            ASSERT(0 == rc);

            if (0 == testStatus) {
                // Touch the 'success' file to tell the parent process we
                // succeeded.

                FD fdSuccess = Obj::open(fileNameSuccess,
                                         Obj::e_OPEN_OR_CREATE,
                                         Obj::e_READ_WRITE);
                Obj::close(fdSuccess);
            }

            // Exit 'main' to avoid doing the cleanup at the end of 'main'
            // twice.

            return testStatus;                                        // RETURN
        }
#else
        if (verbose) {
            cout << "Skipping case 10 on Windows and Cygwin..." << endl;
        }
#endif
      } break;
      case 10: {
        // --------------------------------------------------------------------
        // GETFILESIZELIMIT TEST
        // --------------------------------------------------------------------

        if (verbose) cout << "getFileSizeLimit test\n"
                             "=====================\n";

        Obj::Offset limit = Obj::getFileSizeLimit();

        ASSERT(limit > 0);
        ASSERT(limit > (1LL << 32));

        if (verbose) P(limit);
      } break;
      case 9: {
        // --------------------------------------------------------------------
        // APPEND TEST
        //
        // Concerns:
        //  1. A 'write' puts data at the end of the file when open in append
        //     mode.
        //  2. A 'write' puts data at the end of the file when open in append
        //     mode even after a seek.
        //  3. 'isAppend' is default to 'false'.
        //
        // Plan:
        //  1. Create a file in append mode, write a character, use seek to
        //     change the position of output, write another character, and
        //     verify that the new character is added after the original
        //     character.
        //  2. Reopen the file in append mode, write a character and ensure
        //     that it is added to the end of the file.
        //  3. Reopen the file in normal mode, write a character and ensure
        //     that it overwrites the data in the file instead of appending to
        //     it.
        //
        // Testing:
        //   FD open(const char *p, bool writable, bool exist, bool append)
        // --------------------------------------------------------------------

        bsl::string fileName(tempFileName());

        if (verbose) { P(fileName) }

        Obj::FileDescriptor fd = Obj::open(fileName,
                                           Obj::e_OPEN_OR_CREATE,
                                           Obj::e_READ_APPEND);
        ASSERT(Obj::k_INVALID_FD != fd);

        Obj::write(fd, "A", 1);
        char result[16];

        Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
        ASSERT(1 == Obj::read(fd, result, sizeof result));

        Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
        Obj::write(fd, "B", 1);

        Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
        ASSERT(2 == Obj::read(fd, result, sizeof result));

        Obj::close(fd);

        fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_APPEND);
        Obj::write(fd, "C", 1);
        Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
        ASSERT(3 == Obj::read(fd, result, sizeof result));

        Obj::close(fd);

        fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
        Obj::write(fd, "D", 1);
        Obj::close(fd);

        fd = Obj::open(fileName, Obj::e_OPEN_OR_CREATE, Obj::e_READ_ONLY);
        ASSERT(3 == Obj::read(fd, result, sizeof result));
        Obj::close(fd);

        Obj::remove(fileName);
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // SIMPLE MATCHING TEST
        //
        // Concerns:
        // Unix "glob()", which is called by bdesu_FilesystemUtil::visitPaths,
        // is failing on ibm 64 bit, unfortunately the test driver has not
        // detected or reproduced this error.  This test case is an attempt to
        // get this test driver reproducing the problem.
        //
        // Plan:
        //   Run the usage example 1
        // --------------------------------------------------------------------

        if (verbose) cout << "\nSimple matching test"
                          << "\n====================" << endl;

        const char* dirName = "testDirCase7";
        Obj::remove(dirName, true);
        Obj::createDirectories(dirName, true);
        Obj::setWorkingDirectory(dirName);
        for (int i=0; i<4; ++i) {
            char name[16];
            sprintf(name, "woof.a.%d", i);
            Obj::FileDescriptor fd =
                       Obj::open(name,
                                 Obj::e_OPEN_OR_CREATE,
                                 Obj::e_READ_WRITE);
            Obj::close(fd);
        }

        vector<string> vs;
        Obj::findMatchingPaths(&vs, "woof.a.?");
        sort(vs.begin(), vs.end());

        ASSERT(vs.size() == 4);
        ASSERT(vs[0] == "woof.a.0");
        ASSERT(vs[1] == "woof.a.1");
        ASSERT(vs[2] == "woof.a.2");
        ASSERT(vs[3] == "woof.a.3");

        Obj::setWorkingDirectory("..");
        Obj::remove(dirName, true);
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // TESTING 'getFileSize'
        //
        // Concern: Returns proper file size for the following:
        //   1. A normal file.
        //   2. A normal directory (use empty directory).
        //   3. A file using relative path.
        //   4. A symbolic link (unix only).
        //   5. Non existent file.
        //
        // Plan:
        //   Create the respective files listed in concerns and run
        //   'getFileSize' on it.
        //
        // Testing:
        //   Offset getFileSize(const bsl::string&)
        //   Offset getFileSize(const char *)
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting 'getFileSize'"
                          << "\n=====================" << endl;

        // Setup by first creating a tmp file
        string fileName = tempFileName("tmp.getFileSizeTest.7");
        if (veryVerbose) P(fileName);
        Obj::FileDescriptor fd = Obj::open(fileName,
                                           Obj::e_OPEN_OR_CREATE,
                                           Obj::e_READ_WRITE);
        ASSERT(Obj::k_INVALID_FD != fd);

        const char buffer[] = "testing";
        int bytes           = sizeof buffer;

        Obj::write(fd, buffer, bytes);
        Obj::close(fd);

        // Concern 1

        {
            if (veryVerbose) cout << "\n1. Normal file" << endl;

            Obj::Offset off = Obj::getFileSize(fileName);
            LOOP2_ASSERT(bytes, off, bytes == off);

            Obj::Offset off2 = Obj::getFileSize(fileName.c_str());
            LOOP2_ASSERT(bytes, off2, bytes == off2);

            if (veryVerbose) {
                cout << "Expected " << bytes << endl;
                cout << "Actual ";
                P_(off) P(off2)
            }
        }

        // Concern 2

        {
            if (veryVerbose) cout << "\n2. Normal directory" << endl;

#ifdef BSLS_PLATFORM_OS_WINDOWS
            string dirName("getFileSizeDir");

            // windows directories are 0 sized

            const Obj::Offset EXPECTED = 0;
#else
            string dirName("/tmp/getFileSizeDir");
#endif

            int ret = Obj::createDirectories(dirName, true);
            ASSERT(0 == ret);

            // On UNIX use 'stat64' ('stat' on cygwin) as an oracle: the file
            // size of a directory depends on the file system.

#ifdef BSLS_PLATFORM_OS_CYGWIN
            struct stat oracleInfo;
            int rc = ::stat(dirName.c_str(), &oracleInfo);
            ASSERT(0 == rc);

            Obj::Offset EXPECTED = oracleInfo.st_size;
#elif !defined BSLS_PLATFORM_OS_WINDOWS
            struct stat64 oracleInfo;
            int rc = ::stat64(dirName.c_str(), &oracleInfo);
            ASSERT(0 == rc);

            Obj::Offset EXPECTED = oracleInfo.st_size;
#endif

            Obj::Offset off = Obj::getFileSize(dirName);
            LOOP2_ASSERT(EXPECTED, off, EXPECTED == off);

            Obj::Offset off2 = Obj::getFileSize(dirName.c_str());
            LOOP2_ASSERT(EXPECTED, off2, EXPECTED == off2);

            if (veryVerbose) {
                cout << "Expected " << EXPECTED << endl;
                cout << "Actual ";
                P_(off) P(off2)
            }

            Obj::remove(dirName);
        }

        // Concern 3

        {
            if (veryVerbose) cout << "\n4. Relative Path" << endl;

            string fileName("../getFileSizeTest.txt");
            Obj::FileDescriptor fd = Obj::open(fileName,
                                               Obj::e_OPEN_OR_CREATE,
                                               Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != fd);

            const char buffer[] = "testing";
            int bytes           = sizeof buffer;

            Obj::write(fd, buffer, bytes);
            Obj::close(fd);

            Obj::Offset off = Obj::getFileSize(fileName);
            ASSERT(bytes == off);

            Obj::Offset off2 = Obj::getFileSize(fileName.c_str());
            ASSERT(bytes == off2);

            if (veryVerbose) {
                cout << "Expected " << bytes << endl;
                cout << "Actual ";
                P_(off) P(off2)
            }

            Obj::remove(fileName);
        }

#ifndef BSLS_PLATFORM_OS_WINDOWS
        // Concern 4
        // No symbolic links on windows.

        {
            if (veryVerbose) cout << "\n5. Symbolic Links" << endl;

            bsl::string cmd = "ln -s " + fileName + " testLink";
            system(cmd.c_str());

            string fileName("testLink");
            Obj::Offset off = Obj::getFileSize(fileName);
            ASSERT(bytes == off);

            Obj::Offset off2 = Obj::getFileSize(fileName.c_str());
            ASSERT(bytes == off2);

            if (veryVerbose) {
                cout << "Expected " << bytes << endl;
                cout << "Actual ";
                P_(off) P(off2)
            }

            Obj::remove(fileName);
        }
#endif

        // Concert 5

        {
            if (veryVerbose) cout << "\n6. Non existent file" << endl;

            // Use a random name.

            Obj::Offset off = Obj::getFileSize("acasdf");

            ASSERT(-1 == off);
            if (veryVerbose) {
                cout << "Expected -1" << endl;
                cout << "Actual " << off << endl;
            }
        }

        // Clean up the tmp file.

        Obj::remove(fileName);

      } break;
      case 6: {
        // --------------------------------------------------------------------
        // TESTING 'getAvailableSpace'
        //
        // Concern: 'getAvailableSpace' works.
        //
        // Plan:
        //   Just call the function and check it returns a non-negative result.
        //   We cannot verify it properly.
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting 'getAvailableSpace'"
                          << "\n===========================" << endl;

        Obj::Offset avail = Obj::getAvailableSpace(".");
        if (veryVerbose) {
            cout << "Avail = " << avail << endl;
        }
        ASSERT(0 <= avail);

        string fileName = tempFileName();
        Obj::FileDescriptor fd = Obj::open(fileName,
                                           Obj::e_OPEN_OR_CREATE,
                                           Obj::e_READ_WRITE);
        ASSERT(Obj::k_INVALID_FD != fd);

        avail = Obj::getAvailableSpace(fd);
        if (veryVerbose) {
            cout << "Avail = " << avail << endl;
        }
        ASSERT(0 <= avail);

        Obj::close(fd);
        Obj::remove(fileName);
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // TESTING 'rollFileChain'
        //
        // Concern: Files are rolled.
        //
        // Plan:
        //   Create tmpFile and roll until .1-.3 exist, and verify that they
        //   contain 0-2 (3 rolled off the end).
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting 'rollFileChain' (files)"
                          << "\n===============================" << endl;

        enum { MAXSUFFIX=3 };
        bsl::string tmpFile(tempFileName());
        Obj::FileDescriptor f;

        ASSERT(0 == Obj::rollFileChain(tmpFile, MAXSUFFIX));

        for (int i = MAXSUFFIX; i >= 0; --i) {
            // Create the file - place index into it.

            f = Obj::open(tmpFile,
                          Obj::e_OPEN_OR_CREATE,
                          Obj::e_READ_WRITE);
            LOOP_ASSERT(tmpFile, f != Obj::k_INVALID_FD);
            ASSERT(sizeof(int) == Obj::write(f, &i, sizeof(int)));
            ASSERT(0 == Obj::close(f));

            // Roll the file(s).

            ASSERT(0 == Obj::rollFileChain(tmpFile, MAXSUFFIX));
        }
        ASSERT(0 != Obj::remove(tmpFile)); // does not exist
        tmpFile += ".0";
        int pos = (int) tmpFile.length()-1;

        for (int i = 0; i < MAXSUFFIX; ++i) {
            int value = -1;
            tmpFile[pos] = "123456789"[i];
            f = Obj::open(tmpFile,
                          Obj::e_OPEN,
                          Obj::e_READ_ONLY); // must exist
            LOOP_ASSERT(tmpFile, f != Obj::k_INVALID_FD);
            ASSERT(sizeof(int) == Obj::read(f, &value, sizeof(int)));
            ASSERT(0 == Obj::close(f));
            ASSERT(0 == Obj::remove(tmpFile));
            LOOP2_ASSERT(i, value, i == value);
        }

        if (verbose) {
           cout << "rollFileChain test (directories)" << endl;
        }
        tmpFile = "tmpDir";

        for (int i = MAXSUFFIX; i >= 0; --i) {
            // Create the file - place index into it.

            ASSERT(0 == Obj::createDirectories(tmpFile, true));
            f = Obj::open(tmpFile + PS + "file",
                          Obj::e_OPEN_OR_CREATE,
                          Obj::e_READ_WRITE);
            ASSERT(f != Obj::k_INVALID_FD);
            ASSERT(sizeof(int) == Obj::write(f, &i, sizeof(int)));
            ASSERT(0 == Obj::close(f));

            // Roll the file(s).

            ASSERT(0 == Obj::rollFileChain(tmpFile, MAXSUFFIX));
        }
        ASSERT(0 != Obj::remove(tmpFile, true)); // does not exist
        tmpFile += ".0";
        pos = (int) tmpFile.length()-1;

        for (int i = 0; i < MAXSUFFIX; ++i) {
            int value = -1;
            tmpFile[pos] = "123456789"[i];
            f = Obj::open(tmpFile + PS + "file",
                          Obj::e_OPEN,
                          Obj::e_READ_ONLY);
            ASSERT(f != Obj::k_INVALID_FD);
            ASSERT(sizeof(int) == Obj::read(f, &value, sizeof(int)));
            ASSERT(0 == Obj::close(f));
            ASSERT(0 == Obj::remove(tmpFile, true));
            LOOP2_ASSERT(i, value, i == value);
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // TESTING 'isRegularFile' & 'isDirectory'
        //
        // Concern: These functions work, including on filenames which do not
        //          exist at all.
        //
        // Plan:
        //   Permutate a test vector with a list of different files and
        //   directories to test.
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting 'isRegularFile' & 'isDirectory'"
                          << "\n======================================="
                          << endl;

        struct Parameters {
            const char* good;
            const char* badNoExist;
            const char* badWrongType;
        };

        struct ParametersByType {
            Parameters regular;
            Parameters directory;
        } parameters = {
            { "tmp.filesystemutil.case4" PS "file",
              "tmp.filesystemutil.case4" PS "file2",
              "tmp.filesystemutil.case4" PS "dir"  },
            { "tmp.filesystemutil.case4" PS "dir",
              "tmp.filesystemutil.case4" PS "dir2",
              "tmp.filesystemutil.case4" PS "file" }
        };

        const Parameters& r = parameters.regular;
        const Parameters& d = parameters.directory;

        ASSERT(0 == Obj::createDirectories(r.good));

        makeArbitraryFile(r.good);
        ASSERT(0 == Obj::createDirectories(r.badWrongType, true));
        ASSERT(true == Obj::isRegularFile(r.good));
        ASSERT(false == Obj::isRegularFile(r.badNoExist));
        ASSERT(false == Obj::isRegularFile(r.badWrongType));

        makeArbitraryFile(d.badWrongType);
        ASSERT(0 == Obj::createDirectories(d.good, true));
        ASSERT(true == Obj::isDirectory(d.good));
        ASSERT(false == Obj::isDirectory(d.badNoExist));
        ASSERT(false == Obj::isDirectory(d.badWrongType));

#ifndef BSLS_PLATFORM_OS_WINDOWS
        if (veryVerbose) {
           cout << "...symbolic link tests..." << endl;
        }

        bsl::string absolute;
        ASSERT(0 == Obj::getWorkingDirectory(&absolute));
        bdesu_PathUtil::appendRaw(&absolute, r.good);

        bsl::string link = absolute;
        bdesu_PathUtil::popLeaf(&link);
        bdesu_PathUtil::appendRaw(&link, "link_rg");
        int rc = symlink(absolute.c_str(), link.c_str());

        // test invariant:

        ASSERT(0 == rc);

        ASSERT(false == Obj::isRegularFile(link.c_str()));
        ASSERT(true  == Obj::isRegularFile(link.c_str(), true));

        bsl::string link2 = r.good;
        bdesu_PathUtil::popLeaf(&link2);
        bdesu_PathUtil::appendRaw(&link2, "link_rg2");
        rc = symlink(link.c_str(), link2.c_str());

        // test invariant:

        ASSERT(0 == rc);

        ASSERT(false == Obj::isRegularFile(link2));
        ASSERT(true  == Obj::isRegularFile(link2, true));

        bdesu_PathUtil::popLeaf(&link);
        bdesu_PathUtil::appendRaw(&link, "link_rbw");
        bdesu_PathUtil::popLeaf(&absolute);
        bdesu_PathUtil::popLeaf(&absolute);
        bdesu_PathUtil::appendRaw(&absolute, r.badWrongType);
        rc = symlink(absolute.c_str(), link.c_str());

        // test invariant:

        ASSERT(0 == rc);

        ASSERT(false == Obj::isRegularFile(link));
        ASSERT(false == Obj::isRegularFile(link, true));

        bdesu_PathUtil::popLeaf(&link);
        bdesu_PathUtil::appendRaw(&link, "link_rbn");
        bdesu_PathUtil::popLeaf(&absolute);
        bdesu_PathUtil::popLeaf(&absolute);
        bdesu_PathUtil::appendRaw(&absolute, r.badNoExist);
        rc = symlink(absolute.c_str(), link.c_str());

        // test invariant:

        ASSERT(0 == rc);

        ASSERT(false == Obj::isRegularFile(link));
        ASSERT(false == Obj::isRegularFile(link, true));

        bdesu_PathUtil::popLeaf(&link);
        bdesu_PathUtil::appendRaw(&link, "link_dg");
        bdesu_PathUtil::popLeaf(&absolute);
        bdesu_PathUtil::popLeaf(&absolute);
        bdesu_PathUtil::appendRaw(&absolute, d.good);
        rc = symlink(absolute.c_str(), link.c_str());

        // test invariant:

        ASSERT(0 == rc);

        ASSERT(false == Obj::isDirectory(link));
        ASSERT(false == Obj::isRegularFile(link));
        ASSERT(true  == Obj::isDirectory(link, true));
        ASSERT(false == Obj::isRegularFile(link, true));

        bdesu_PathUtil::popLeaf(&link2);
        bdesu_PathUtil::appendRaw(&link2, "link_dg2");
        rc = symlink(link.c_str(), link2.c_str());

        // test invariant:

        ASSERT(0 == rc);

        ASSERT(false == Obj::isDirectory(link2));
        ASSERT(false == Obj::isRegularFile(link2));
        ASSERT(true  == Obj::isDirectory(link2, true));
        ASSERT(false == Obj::isRegularFile(link2, true));

#endif  // Symbolic link testing on non-Windows

#ifndef BSLS_PLATFORM_OS_WINDOWS  // (unix domain socket)
        {
            // Unix domain sockets should return 'false' for 'isRegularFile'
            // and 'isDirectory' (DRQS 2071065).

            if (veryVerbose) {
                cout << "...unix domain socket..." << endl;
            }
            bsl::string filename = tempFileName();
            Obj::remove(filename);

            int socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
            LOOP_ASSERT(socketFd, socketFd >= 0);

            struct sockaddr_un address;
            address.sun_family = AF_UNIX;
            sprintf(address.sun_path, "%s", filename.c_str());

            // Add one to account for the null terminator for the filename.

            const int ADDR_LEN = (int) (sizeof(address.sun_family) +
                                        filename.size() +
                                        1);

            int rc = bind(socketFd, (struct sockaddr *)&address, ADDR_LEN);
            LOOP3_ASSERT(rc, errno, strerror(errno), 0 == rc);


            LOOP_ASSERT(filename, Obj::exists(filename));
            LOOP_ASSERT(filename, !Obj::isDirectory(filename));
            LOOP_ASSERT(filename, !Obj::isRegularFile(filename));
            Obj::remove(filename);
        }
#endif  // BSLS_PLATFORM_OS_WINDOWS (unix domain socket)

        //clean up

       ASSERT(0 == Obj::remove("tmp.filesystemutil.case4",
                                                true));
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // TESTING pattern matching
        //
        // Concern: Both '*' and '?' characters are supported, and
        //          can appear in multiple directories in the path.
        //
        // Plan:
        //   Make sure both '*' and '?' characters are supported with
        //   'findMatchingPath'
        // --------------------------------------------------------------------

        if (verbose) cout << "\nTesting pattern matching"
                          << "\n========================" << endl;

        const char* filenames[] = {
           "abcd",
           "zyx",
           "zy.z",
           "abc.zzz",
           "abc.def",
        };

#define PATH  "bdesu_filesystemutil.temp.3.futc3"
#define PATHQ "bdesu_filesystemutil.temp.3.futc?"

        bsl::string path(PATH);

        ASSERT(Obj::remove(path.c_str(), true));

        // The string literal "futc3/b???/*d*" seems to confuse the
        // Sun compiler, which complains about the character sequence "\*".
        // So let's hard-code it.

        const char tripleQMarkLiteral[] = {'b','d','e','s','u','_','f','i','l',
                                           'e','s','y','s','t','e','m','u','t',
                                           'i','l','.','t','e','m','p','.','3',
                                           '.','f','u','t','c','3','/','b',
                                           '?','?','?','/','*','d','*', 0};

        struct Parameters {
            int         line;
            const char* pattern;
            const char* result;
        } parameters[] = {
            {L_, "", ""},
            {L_, PATH "/*/*foo*", ""},
            {L_, PATH "/*/*d*", PATH "/alpha/abc.def:" PATH "/alpha/abcd:"
                            PATH "/beta/abc.def:" PATH "/beta/abcd"},
            {L_, tripleQMarkLiteral, PATH "/beta/abc.def:" PATH "/beta/abcd"},
            {L_, PATH "/*b*", PATH "/beta"},
#ifdef BSLS_PLATFORM_OS_WINDOWS
            {L_, PATH "/*b*/*.?",
                      PATH "/beta/abcd:" PATH "/beta/zy.z:" PATH "/beta/zyx" },
            {L_, PATHQ "/*b*/*.?",
                       PATH "/beta/abcd:" PATH "/beta/zy.z:" PATH "/beta/zyx"},
            {L_, PATHQ "/*/abcd.*",PATH "/alpha/abcd:" PATH "/beta/abcd"},
            {L_, PATHQ "/*b*/*.*", PATH "/beta/abc.def:" PATH "/beta/abc.zzz:"
                              PATH "/beta/abcd:" PATH "/beta/zy.z:"
                              PATH "/beta/zyx"},
            {L_, PATH "*/*/*.?",
               PATH "/alpha/abcd:" PATH "/alpha/zy.z:" PATH "/alpha/zyx:"
               PATH "/beta/abcd:" PATH "/beta/zy.z:" PATH "/beta/zyx"}
#else
            {L_, PATH "/*b*/*.?", PATH "/beta/zy.z"},
            {L_, PATHQ "/*b*/*.?", PATH "/beta/zy.z"},
            {L_, PATHQ "/*/abcd.*", ""},
            {L_, PATHQ "/*b*/*.*",
                   PATH "/beta/abc.def:" PATH "/beta/abc.zzz:"
                   PATH "/beta/zy.z"},
            {L_, PATH "*/*/*.?", PATH "/alpha/zy.z:" PATH "/beta/zy.z"}
#endif
        };

        const int numFiles = sizeof(filenames) / sizeof(*filenames);

        bdesu_PathUtil::appendRaw(&path, "alpha");

        if (veryVerbose) { T_() cout << "Creating directory "; P(path) }

        ASSERT(0 == Obj::createDirectories(path.c_str(), true));
        for (int i = 0; i < numFiles; ++i) {
            bdesu_PathUtil::appendRaw(&path, filenames[i]);

            if (veryVerbose) { T_() T_() cout << "Creating file "; P(path) }

            Obj::FileDescriptor f =  Obj::open(path,
                                               Obj::e_CREATE,
                                               Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != f);
            ASSERT(0 == Obj::close(f));

            if (veryVerbose) { T_() T_() cout << "Looking up file "; P(path) }

            vector<bsl::string> lookup;
            Obj::findMatchingPaths(&lookup, path.c_str());
            string rollup = rollupPaths(lookup);
#ifdef BSLS_PLATFORM_OS_WINDOWS
            replace_if(rollup.begin(), rollup.end(), isForwardSlash, *PS);
#endif
            LOOP2_ASSERT(path, rollup, path == rollup);

            bdesu_PathUtil::popLeaf(&path);
        }
        bdesu_PathUtil::popLeaf(&path);

        bdesu_PathUtil::appendRaw(&path, "beta");

        if (veryVerbose) { T_() cout << "Creating directory "; P(path) }

        ASSERT(0 == Obj::createDirectories(path.c_str(), true));
        for (int i = 0; i < numFiles; ++i) {
            bdesu_PathUtil::appendRaw(&path, filenames[i]);

            if (veryVerbose) { T_() T_() cout << "Creating file "; P(path) }

            Obj::FileDescriptor f = Obj::open(path,
                                              Obj::e_CREATE,
                                              Obj::e_READ_WRITE);
            ASSERT(Obj::k_INVALID_FD != f);
            ASSERT(0 == Obj::close(f));

            if (veryVerbose) { T_() T_() cout << "Looking up "; P(path) }

            vector<bsl::string> lookup;
            Obj::findMatchingPaths(&lookup, path.c_str());
            string rollup = rollupPaths(lookup);
#ifdef BSLS_PLATFORM_OS_WINDOWS
            replace_if(rollup.begin(), rollup.end(), isForwardSlash, *PS);
#endif
            LOOP2_ASSERT(path, rollup, path == rollup);

            bdesu_PathUtil::popLeaf(&path);
        }
        bdesu_PathUtil::popLeaf(&path);

        vector<bsl::string> resultPaths;
        enum { NUM_PARAMETERS = sizeof(parameters) / sizeof(*parameters) };
        for (int i = 0; i < NUM_PARAMETERS; ++i) {
            const Parameters& p = parameters[i];
            const int LINE      =  p.line;

            string pattern(p.pattern);
#ifdef BSLS_PLATFORM_OS_WINDOWS
            replace_if(pattern.begin(), pattern.end(), isForwardSlash, *PS);
#endif

            if (veryVerbose) { T_() T_() cout << "Looking up "; P(path) }

            Obj::findMatchingPaths(&resultPaths, pattern.c_str());
            string rollup = rollupPaths(resultPaths);
            LOOP3_ASSERT(LINE, p.result, rollup, string(p.result) == rollup);
        }

        ASSERT(0 == Obj::remove(path.c_str(), true));
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // OPEN TEST
        //
        // Concerns:
        //: 1 (For deprecated overload) A 'write' puts data at the end of the
        //:   file when open in append mode.
        //:
        //: 2 (For deprecated overload) A 'write' puts data at the end of the
        //:   file when open in append mode even after a seek.
        //:
        //: 3 (For deprecated overload) 'isAppend' is default to 'false'.
        //:
        //: 4 An existing file can be opened when 'OPEN' is specified, and a
        //:   non-existent file causes an error when 'OPEN' is specified.
        //:
        //: 5 An existing file causes an error when 'CREATE' is specified, and
        //:   a non-existent file can be opened when 'CREATE' is specified.
        //:
        //: 6 An existing file can be opened when 'OPEN_OR_CREATE' is
        //:   specified, and a non-existent file can be opened when
        //:   'OPEN_OR_CREATE' is specified.
        //:
        //: 7 Reading is possible when 'READ_ONLY' is specified, and writing is
        //:   impossible when 'READ_ONLY' is specified.
        //:
        //: 8 Reading is impossible when 'WRITE_ONLY' is specified, and writing
        //:   is possible when 'WRITE_ONLY' is specified.
        //:
        //: 9 Reading is impossible when 'APPEND_ONLY' is specified, and
        //:   writing is possible when 'APPEND_ONLY' is specified.
        //:
        //:10 Reading is possible when 'READ_WRITE' is specified, and writing
        //:   is possible when 'READ_WRITE' is specified.
        //:
        //:11 Reading is possible when 'READ_APPEND' is specified, and writing
        //:   is possible when 'READ_APPEND' is specified.
        //:
        //:12 When either 'APPEND_ONLY' or 'READ_APPEND' is specified, all
        //:   writes are made at the end of file.
        //:
        //:13 When 'TRUNCATE' is specified, the previous contents of the file,
        //:   if any, are discarded.
        //:
        //:14 When 'KEEP' is specified, the previous contents of the file, if
        //:   any, are preserved.
        //
        // Plan:
        //: 1 Create a file in append mode, write a character, use seek to
        //:   change the position of output, write another character, and
        //:   verify that the new character is added after the original
        //:   character.
        //:
        //: 2 Reopen the file in append mode, write a character and ensure that
        //:   it is added to the end of the file.
        //:
        //: 3 Reopen the file in normal mode, write a character and ensure that
        //:   it overwrites the data in the file instead of appending to it.
        //:
        //: 4  Note that the !EXISTS case should be tested before the EXISTS
        //: cases, so that we can establish that 'open' is capable of creating
        //: files before we create files to prepare for subsequent tests.
        //
        // Testing:
        //   FD open(const char *path, openPolicy, ioPolicy, truncatePolicy)
        //   FD open(const string& path, openPolicy, ioPolicy, truncatePolicy)
        // --------------------------------------------------------------------

        if (verbose) cout << endl
                          << "OPEN" << endl
                          << "====" << endl;

        enum {
            k_READ_BUFFER_SIZE = 64
        };

        if (verbose) cout << "\nBootstrap, phase 1: 'open' file mode."
                          << endl;
        {
            const Obj::FileOpenPolicy OP = Obj::e_OPEN;
            const Obj::FileOpenPolicy CR = Obj::e_CREATE;
            const Obj::FileOpenPolicy OC = Obj::e_OPEN_OR_CREATE;

            const Obj::FileIOPolicy RO = Obj::e_READ_ONLY;
            const Obj::FileIOPolicy WO = Obj::e_WRITE_ONLY;
            const Obj::FileIOPolicy AO = Obj::e_APPEND_ONLY;
            const Obj::FileIOPolicy RW = Obj::e_READ_WRITE;
            const Obj::FileIOPolicy RA = Obj::e_READ_APPEND;

            const Obj::FileTruncatePolicy TR = Obj::e_TRUNCATE;
            const Obj::FileTruncatePolicy KP = Obj::e_KEEP;

            const struct DataRow {
                int d_line;
                Obj::FileOpenPolicy d_openPolicy;
                Obj::FileIOPolicy d_ioPolicy;
                Obj::FileTruncatePolicy d_initPolicy;
                bool d_exists;
                bool d_success;
            } DATA[] = {
                //LN openPolicy ioPolicy initPolicy exists success
                //-- ---------- -------- ---------- ------ -------
                {L_, CR,        WO,      KP,       false,   true },
                {L_, CR,        WO,      KP,        true,  false },
                {L_, OP,        WO,      KP,       false,  false },
                {L_, OP,        WO,      KP,        true,   true },
                {L_, OC,        WO,      KP,       false,   true },
                {L_, OC,        WO,      KP,        true,   true },

                {L_, CR,        RO,      KP,       false,   true },
                {L_, CR,        RO,      KP,        true,  false },
                {L_, OP,        RO,      KP,       false,  false },
                {L_, OP,        RO,      KP,        true,   true },
                {L_, OC,        RO,      KP,       false,   true },
                {L_, OC,        RO,      KP,        true,   true },

                {L_, CR,        AO,      KP,       false,   true },
                {L_, CR,        AO,      KP,        true,  false },
                {L_, OP,        AO,      KP,       false,  false },
                {L_, OP,        AO,      KP,        true,   true },
                {L_, OC,        AO,      KP,       false,   true },
                {L_, OC,        AO,      KP,        true,   true },

                {L_, CR,        RW,      KP,       false,   true },
                {L_, CR,        RW,      KP,        true,  false },
                {L_, OP,        RW,      KP,       false,  false },
                {L_, OP,        RW,      KP,        true,   true },
                {L_, OC,        RW,      KP,       false,   true },
                {L_, OC,        RW,      KP,        true,   true },

                {L_, CR,        RA,      KP,       false,   true },
                {L_, CR,        RA,      KP,        true,  false },
                {L_, OP,        RA,      KP,       false,  false },
                {L_, OP,        RA,      KP,        true,   true },
                {L_, OC,        RA,      KP,       false,   true },
                {L_, OC,        RA,      KP,        true,   true },

                {L_, CR,        WO,      TR,       false,   true },
                {L_, CR,        WO,      TR,        true,  false },
                {L_, OP,        WO,      TR,       false,  false },
                {L_, OP,        WO,      TR,        true,   true },
                {L_, OC,        WO,      TR,       false,   true },
                {L_, OC,        WO,      TR,        true,   true },

                {L_, CR,        RO,      TR,       false,   true },
                {L_, CR,        RO,      TR,        true,  false },
                {L_, OP,        RO,      TR,       false,  false },
                // Truncate without Write or Create disallowed:
                {L_, OP,        RO,      TR,        true,  false },
                {L_, OC,        RO,      TR,       false,   true },
                {L_, OC,        RO,      TR,        true,   true },

                {L_, CR,        AO,      TR,       false,   true },
                {L_, CR,        AO,      TR,        true,  false },
                {L_, OP,        AO,      TR,       false,  false },
                // Truncate without Write or Create disallowed:
                {L_, OP,        AO,      TR,        true,  false },
                {L_, OC,        AO,      TR,       false,   true },
                {L_, OC,        AO,      TR,        true,   true },

                {L_, CR,        RW,      TR,       false,   true },
                {L_, CR,        RW,      TR,        true,  false },
                {L_, OP,        RW,      TR,       false,  false },
                {L_, OP,        RW,      TR,        true,   true },
                {L_, OC,        RW,      TR,       false,   true },
                {L_, OC,        RW,      TR,        true,   true },

                {L_, CR,        RA,      TR,       false,   true },
                {L_, CR,        RA,      TR,        true,  false },
                {L_, OP,        RA,      TR,       false,  false },
                // Truncate without Write or Create disallowed:
                {L_, OP,        RA,      TR,        true,  false },
                {L_, OC,        RA,      TR,       false,   true },
                {L_, OC,        RA,      TR,        true,   true },
            };
            int NUM_DATA = sizeof(DATA) / sizeof(DataRow);

            bool isCreateFileTested = false;

            bsl::string fileName(tempFileName());

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int                  LINE        = DATA[ti].d_line;
                const bool                 EXISTS      = DATA[ti].d_exists;
                const bool                 EXP_SUCCESS = DATA[ti].d_success;

                const Obj::FileOpenPolicy     OPEN_POLICY
                                                       = DATA[ti].d_openPolicy;
                const Obj::FileIOPolicy       IO_POLICY
                                                       = DATA[ti].d_ioPolicy;
                const Obj::FileTruncatePolicy INIT_POLICY
                                                       = DATA[ti].d_initPolicy;

                if (veryVerbose) {
                    T_()
                    P_(OPEN_POLICY) P_(IO_POLICY) P_(INIT_POLICY) P(EXISTS)
                }

                // Establish a consistent baseline: 'fileName' does not exist.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }

                LOOP2_ASSERT(LINE, fileName, !Obj::exists(fileName));

                // If this test expects 'fileName' to exist, it must be created
                // using the (already-tested) 'e_CREATE' open mode.

                if (EXISTS) {
                    // We cannot use the combination of 'e_CREATE' with
                    // 'e_WRITE_ONLY' until it has been tested.

                    LOOP_ASSERT(LINE, isCreateFileTested);

                    Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_WRITE_ONLY);
                    LOOP_ASSERT(LINE, Obj::k_INVALID_FD != fd);

                    Obj::close(fd);

                    LOOP2_ASSERT(LINE, fileName, Obj::exists(fileName));
                    LOOP_ASSERT(LINE, 0 == Obj::getFileSize(fileName));
                }

                // Test open success

                Obj::FileDescriptor fd = Obj::open(
                                fileName, OPEN_POLICY, IO_POLICY, INIT_POLICY);
                LOOP3_ASSERT(LINE, EXP_SUCCESS, fd,
                                     EXP_SUCCESS == (Obj::k_INVALID_FD != fd));

                if (Obj::k_INVALID_FD == fd) {
                    // Expected failure.  We can do no more testing with an
                    // invalid file descriptor, so go on to the next test.

                    continue;
                }

                // File Descriptor Attribute Testing

#ifndef BSLS_PLATFORM_OS_WINDOWS
                switch (IO_POLICY) {
                  case Obj::e_READ_ONLY: {
                    int flags = fcntl(fd, F_GETFL);
                    LOOP2_ASSERT(LINE, IO_POLICY,
                                               O_RDONLY == (flags & O_RDONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_WRONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_APPEND));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_RDWR));
                  } break;
                  case Obj::e_WRITE_ONLY: {
                    int flags = fcntl(fd, F_GETFL);
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_RDONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY,
                                               O_WRONLY == (flags & O_WRONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_APPEND));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_RDWR));
                  } break;
                  case Obj::e_APPEND_ONLY: {
                    int flags = fcntl(fd, F_GETFL);
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_RDONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY,
                                               O_WRONLY == (flags & O_WRONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY,
                                               O_APPEND == (flags & O_APPEND));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_RDWR));
                  } break;
                  case Obj::e_READ_WRITE: {
                    int flags = fcntl(fd, F_GETFL);
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_RDONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_WRONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_APPEND));
                    LOOP2_ASSERT(LINE, IO_POLICY, O_RDWR == (flags & O_RDWR));
                  } break;
                  case Obj::e_READ_APPEND: {
                    int flags = fcntl(fd, F_GETFL);
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_RDONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY, 0 == (flags & O_WRONLY));
                    LOOP2_ASSERT(LINE, IO_POLICY,
                                               O_APPEND == (flags & O_APPEND));
                    LOOP2_ASSERT(LINE, IO_POLICY, O_RDWR == (flags & O_RDWR));
                  } break;
                  default: {
                      ASSERT(false);
                  }
                }
#endif

                if (   Obj::e_CREATE     == OPEN_POLICY
                    && Obj::e_WRITE_ONLY == IO_POLICY  ) {
                    isCreateFileTested = true;
                }

                Obj::close(fd);
                Obj::remove(fileName);
            }
        }

        if (verbose) cout << "\nBootstrap, phase 2: basic IO test."
                          << endl;
        {
            bsl::string fileName(tempFileName());

            if (Obj::exists(fileName)) {
                Obj::remove(fileName);
            }
            LOOP_ASSERT(fileName, !Obj::exists(fileName));

            Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
            LOOP_ASSERT(fileName, Obj::k_INVALID_FD != fd);

            const char blockA[]   = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
            const int  numBytesA  =
                            static_cast<int>(sizeof(blockA) / sizeof(*blockA));
            const char blockB[]   = { '0', '1', '2', '3', '4' };
            const int  numBytesB  =
                            static_cast<int>(sizeof(blockB) / sizeof(*blockB));

            char buffer[k_READ_BUFFER_SIZE];

#ifndef BSLS_PLATFORM_OS_WINDOWS
            // e_READ_WRITE supports writing

            ASSERT(0 == lseek(fd, 0, SEEK_END));
            ASSERT(numBytesA == write(fd, blockA, numBytesA));
            ASSERT(numBytesA == lseek(fd, 0, SEEK_CUR));
            ASSERT(numBytesA == lseek(fd, 0, SEEK_END));

            // e_READ_WRITE supports reading

            ASSERT(0 == lseek(fd, 0, SEEK_SET));
            memset(buffer, 0, k_READ_BUFFER_SIZE);
            int numBytes =
                        static_cast<int>(read(fd, buffer, k_READ_BUFFER_SIZE));
            LOOP2_ASSERT(numBytes, numBytesA, numBytes == numBytesA);
            LOOP2_ASSERT(blockA, buffer,
                                       0 == memcmp(blockA, buffer, numBytesA));

            // e_READ_WRITE write is not append

            ASSERT(0 == lseek(fd, 0, SEEK_SET));
            ASSERT(numBytesB == write(fd, blockB, numBytesB));
            LOOP2_ASSERT(blockA, blockB, numBytesA > numBytesB);
            ASSERT(numBytesB == lseek(fd, 0, SEEK_CUR));
            ASSERT(numBytesA == lseek(fd, 0, SEEK_END));
#else
            // e_READ_WRITE supports writing

            ASSERT(0 == SetFilePointer(fd, 0, NULL, FILE_END));
            DWORD numBytes;
            ASSERT(WriteFile(fd, blockA, numBytesA, &numBytes, NULL));
            LOOP2_ASSERT(numBytes, numBytesA, numBytes == numBytesA);
            ASSERT(numBytesA == SetFilePointer(fd, 0, NULL, FILE_CURRENT));
            ASSERT(numBytesA == SetFilePointer(fd, 0, NULL, FILE_END));

            // e_READ_WRITE supports reading

            ASSERT(0 == SetFilePointer(fd, 0, NULL, FILE_BEGIN));
            memset(buffer, 0, k_READ_BUFFER_SIZE);
            ASSERT(ReadFile(fd, buffer, k_READ_BUFFER_SIZE, &numBytes, NULL));
            LOOP2_ASSERT(numBytes, numBytesA, numBytes == numBytesA);
            LOOP2_ASSERT(blockA, buffer,
                                       0 == memcmp(blockA, buffer, numBytesA));

            // e_READ_WRITE write is not append

            ASSERT(0 == SetFilePointer(fd, 0, NULL, FILE_BEGIN));
            ASSERT(WriteFile(fd, blockB, numBytesB, &numBytes, NULL));
            LOOP2_ASSERT(blockA, blockB, numBytesA > numBytesB);
            LOOP2_ASSERT(numBytes, numBytesB, numBytes == numBytesB);
            ASSERT(numBytesB == SetFilePointer(fd, 0, NULL, FILE_CURRENT));
            ASSERT(numBytesA == SetFilePointer(fd, 0, NULL, FILE_END));
#endif  // BSLS_PLATFORM_OS_WINDOWS

            Obj::close(fd);
            Obj::remove(fileName);
        }

        if (verbose) cout <<
                            "\nBootstrap, phase 3: 'read' and 'write'."
                          << endl;
        {
            const char blockA[]   = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
            const int  numBytesA  =
                            static_cast<int>(sizeof(blockA) / sizeof(*blockA));
            const char blockB[]   = { '0', '1', '2', '3', '4' };
            const int  numBytesB  =
                            static_cast<int>(sizeof(blockB) / sizeof(*blockB));
            int        lengths[]  = { 0, 1, 2, 3, 4, 5, 6 };
            const int  numLengths =
                          static_cast<int>(sizeof(lengths) / sizeof(*lengths));
            char       buffer[k_READ_BUFFER_SIZE];

            bsl::string fileName(tempFileName());

            if (veryVerbose) { cout << "\tBasic write" << endl; }

            // Basic write (lengths 0, 1, N)

            for (int i = 0; i < numLengths; ++i) {
                if (veryVerbose) { T_() P_(i) P(fileName) }

                // Write data to empty file.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP2_ASSERT(lengths[i], fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                int bytesWritten = Obj::write(fd, blockA, lengths[i]);
                if (0 <= lengths[i]) {
                    LOOP2_ASSERT(lengths[i], bytesWritten,
                                                   lengths[i] == bytesWritten);
                }
                else {
                    LOOP2_ASSERT(lengths[i], bytesWritten, 0 > bytesWritten);
                }

                // Confirm file has expected size.

                Obj::close(fd);
                if (0 <= lengths[i]) {
                    LOOP2_ASSERT(lengths[i], bytesWritten,
                                   bytesWritten == Obj::getFileSize(fileName));
                }
                Obj::remove(fileName);
            }

            // Write data to empty file.

            {
                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                Obj::write(fd, blockA, numBytesA);

                Obj::close(fd);
            }

            if (veryVerbose) { cout << "\tBasic read" << endl; }

            // Basic read (lengths 0, 1, N)

            for (int i = 0; i < numLengths; ++i) {
                if (veryVerbose) { T_() P_(i) P(fileName) }

                LOOP2_ASSERT(lengths[i], fileName, Obj::exists(fileName));

                // Read data from file.
                // A: Read amount requested.

                Obj::FileDescriptor fd = Obj::open(
                                     fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                int bytesRead = Obj::read(fd, buffer, lengths[i]);

                // Confirm length of data.

                LOOP2_ASSERT(lengths[i], bytesRead, lengths[i] == bytesRead);

                // Confirm contents of data.

                LOOP_ASSERT(lengths[i],
                                       0 == memcmp(buffer, blockA, bytesRead));

                Obj::close(fd);
            }

            {
                // B: Read until file is exhausted.

                Obj::FileDescriptor fd = Obj::open(
                                     fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                int bytesRead = Obj::read(fd, buffer, numBytesA + 1);

                // Confirm length of data.

                LOOP2_ASSERT(numBytesA, bytesRead, numBytesA == bytesRead);

                // Confirm contents of data.

                ASSERT(0 == memcmp(buffer, blockA, bytesRead));

                Obj::close(fd);
            }

            Obj::remove(fileName);

            if (veryVerbose) {
                cout << "\tPositional properties of 'write'"
                     << endl;
            }

            // Positional properties of write

            // First write in existing file is at beginning.

            {
                // Prepare file with existing data.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                Obj::write(fd, blockA, numBytesA);

                Obj::close(fd);

                //   Re-open file and write data.

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                int bytesWritten = Obj::write(fd, blockB, numBytesB);
                ASSERT(numBytesB == bytesWritten);

                Obj::close(fd);

                //   Confirm data is written to start of file.

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                memset(buffer, 0, numBytesB);
                int bytesRead = Obj::read(fd, buffer, numBytesB);
                LOOP_ASSERT(bytesRead, 0 < bytesRead);

                LOOP_ASSERT(buffer[0], blockB[0] == buffer[0]);

                Obj::close(fd);
                Obj::remove(fileName);
            }

            // Write moves write position.

            for (int i = 0; i < numLengths; ++i) {
                if (veryVerbose) { T_() P_(i) P(fileName) }

                // Create new file

                // Write data of varying sizes

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP2_ASSERT(lengths[i], fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                int bytesWritten = Obj::write(fd, blockA, lengths[i]);
                LOOP2_ASSERT(lengths[i], bytesWritten,
                                                   lengths[i] == bytesWritten);

                //   Write marker.

                bytesWritten = Obj::write(fd, blockB, 1);
                LOOP2_ASSERT(lengths[i], bytesWritten,
                        bytesWritten == 1);
                Obj::close(fd);

                //   Confirm marker is in expected location.

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                memset(buffer, 0, sizeof(buffer));
                int bytesRead = Obj::read(fd, buffer, lengths[i] + 1);
                LOOP2_ASSERT(lengths[i], bytesRead,
                                                  lengths[i] + 1 == bytesRead);

                for (int j = 0; j < lengths[i]; ++j) {
                    LOOP2_ASSERT(lengths[i], j, blockB[0] != buffer[j]);
                }
                LOOP2_ASSERT(lengths[i], buffer[lengths[i]],
                                              blockB[0] == buffer[lengths[i]]);

                Obj::close(fd);
                Obj::remove(fileName);
            }

            if (veryVerbose) {
                cout << "\tPositional properties of 'read'"
                     << endl;
            }

            // Positional properties of read

            // Prepare file with existing data.
            {
                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                Obj::write(fd, blockA, numBytesA);

                Obj::close(fd);
            }

            for (int i = 0; i < numLengths; ++i) {
                if (veryVerbose) { T_() P_(i) P(fileName) }

                //   Re-open file and read data of varying sizes
                Obj::FileDescriptor fd = Obj::open(
                                     fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                int bytesRead = Obj::read(fd, buffer, lengths[i]);
                LOOP2_ASSERT(lengths[i], bytesRead, lengths[i] == bytesRead);

                //   Then read marker.
                memset(buffer, 0, sizeof(buffer));
                bytesRead = Obj::read(fd, buffer, 1);
                LOOP2_ASSERT(lengths[i], bytesRead, 1 == bytesRead);

                //   Confirm marker is expected.
                LOOP2_ASSERT(lengths[i], buffer[0],
                                              blockA[lengths[i]] == buffer[0]);

                Obj::close(fd);

                //   Re-open file and read until near end of file.

                int fileSize = static_cast<int>(Obj::getFileSize(fileName));
                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                bytesRead = Obj::read(fd, buffer, fileSize - lengths[i]);
                LOOP3_ASSERT(lengths[i], fileSize, bytesRead,
                                           fileSize - lengths[i] == bytesRead);

                //   Read data past end of file.
                memset(buffer, 0, sizeof(buffer));
                bytesRead = Obj::read(fd, buffer, (lengths[i] * 2) + 1);

                //   Confirm read failed to read all data.
                LOOP3_ASSERT(lengths[i], fileSize, bytesRead, 0 <= bytesRead);
                LOOP3_ASSERT(lengths[i], fileSize, bytesRead,
                                             (lengths[i] * 2) + 1 > bytesRead);
                if (0 < bytesRead) {
                    LOOP3_ASSERT(lengths[i], fileSize, bytesRead,
                                   blockA[fileSize - lengths[i]] == buffer[0]);
                }

                Obj::close(fd);
            }

            Obj::remove(fileName);

            if (veryVerbose) {
                cout << "\tCombined positional properties"
                     << endl;
            }

            // Combined positional properties

            if (veryVerbose) {
                cout << "\tWrite followed by read"
                     << endl;
            }

            // Write followed by read.

            for (int i = 0; i < numLengths; ++i) {
                if (veryVerbose) { T_() P_(i) P(fileName) }

                // Prepare file with existing data.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                Obj::write(fd, blockA, numBytesA);

                Obj::close(fd);

                //  Re-open file and write data of various lengths.

                LOOP2_ASSERT(lengths[i], fileName, Obj::exists(fileName));

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                int bytesWritten = Obj::write(fd, blockA, lengths[i]);
                LOOP2_ASSERT(lengths[i], bytesWritten,
                                                   bytesWritten == lengths[i]);

                //  Read data.

                memset(buffer, 0, sizeof(buffer));
                int bytesRead = Obj::read(fd, buffer, 1);
                LOOP2_ASSERT(lengths[i], bytesRead, 1 == bytesRead);

                //  Confirm that read occurred at end position of write.

                LOOP2_ASSERT(lengths[i], buffer[0],
                                              buffer[0] == blockA[lengths[i]]);

                Obj::close(fd);
                Obj::remove(fileName);
            }

            if (veryVerbose) {
                cout << "\tRead followed by write"
                     << endl;
            }

            // Read followed by write.

            for (int i = 0; i < numLengths; ++i) {
                if (veryVerbose) { T_() P_(i) P(fileName) }

                // Prepare file with existing data.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                Obj::write(fd, blockA, numBytesA);

                Obj::close(fd);

                //  Re-open file and read data of various lengths.

                LOOP2_ASSERT(lengths[i], fileName, Obj::exists(fileName));

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                int bytesRead = Obj::read(fd, buffer, lengths[i]);
                LOOP2_ASSERT(lengths[i], bytesRead, bytesRead == lengths[i]);

                //  Write marker.

                int bytesWritten = Obj::write(fd, blockB, 1);
                LOOP2_ASSERT(lengths[i], bytesWritten, bytesWritten == 1);

                Obj::close(fd);

                //  Confirm that write began at end position of read.

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                LOOP_ASSERT(lengths[i], Obj::k_INVALID_FD != fd);

                memset(buffer, 0, sizeof(buffer));
                bytesRead = Obj::read(fd, buffer, lengths[i] + 1);
                LOOP2_ASSERT(lengths[i], bytesRead,
                                                  lengths[i] + 1 == bytesRead);

                for (int j = 0; j < lengths[i]; ++j) {
                    LOOP2_ASSERT(lengths[i], j, blockB[0] != buffer[j]);
                }
                LOOP2_ASSERT(lengths[i], buffer[lengths[i]],
                                              blockB[0] == buffer[lengths[i]]);

                Obj::close(fd);
                Obj::remove(fileName);
            }

            // Negative testing
            if (verbose) cout << "\nNegative testing" << endl;
            {
                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                bsls::AssertFailureHandlerGuard hG(
                                             bsls::AssertTest::failTestDriver);

                ASSERT_PASS(Obj::read(fd, buffer,  1));
                ASSERT_PASS(Obj::read(fd, buffer,  0));
                ASSERT_FAIL(Obj::read(fd, buffer, -1));
                ASSERT_FAIL(Obj::read(fd, 0,       1));

                ASSERT_PASS(Obj::write(fd, blockA,  1));
                ASSERT_PASS(Obj::write(fd, blockA,  0));
                ASSERT_FAIL(Obj::write(fd, blockA, -1));
                ASSERT_FAIL(Obj::write(fd, 0,       1));

                Obj::close(fd);
                Obj::remove(fileName);
            }
        }

        if (verbose) cout << "\nBootstrap, phase 4: 'seek'."
                          << endl;
        {
            const char blockA[]   = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
            const int  numBytesA  =
                            static_cast<int>(sizeof(blockA) / sizeof(*blockA));
            const char blockB[]   = { '0', '1', '2', '3', '4' };
            int        lengths[]  = { 0, 1, 2, 3, 4, 5, 6 };
            const int  numLengths =
                          static_cast<int>(sizeof(lengths) / sizeof(*lengths));
            char       buffer[k_READ_BUFFER_SIZE];

            bsl::string fileName(tempFileName());

            if (veryVerbose) { cout << "\tSeek in empty file." << endl; }

            // Seek in empty file.
            {
                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                // Confirm that position starts at 0.

                Obj::Offset position =
                                    Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT);
                LOOP_ASSERT(position, 0 == position);

                // Confirm that seek to begin succeeds -> 0.

                position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
                LOOP_ASSERT(position, 0 == position);

                // Confirm that seek to end succeeds -> 0.

                position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_END);
                LOOP_ASSERT(position, 0 == position);

                // Confirm that seek to non-0 succeeds.

                position = Obj::seek(fd, 1, Obj::e_SEEK_FROM_BEGINNING);
                LOOP_ASSERT(position, 1 == position);

                // Confirm that seek to negative offset fails.

                position = Obj::seek(fd, -1, Obj::e_SEEK_FROM_BEGINNING);
                LOOP_ASSERT(position, -1 == position);

                Obj::close(fd);
            }

            if (veryVerbose) { cout << "\tSeek in non-empty file." << endl; }

            // Seek in non-empty file.

            {
                // Prepare file with existing data.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                Obj::write(fd, blockA, numBytesA);

                Obj::close(fd);

                //   Re-open file and seek.

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                // Confirm that position starts at 0.
                Obj::Offset position =
                                    Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT);
                LOOP_ASSERT(position, 0 == position);

                for (int i = 0; i < numLengths; ++i) {
                    // Confirm that seek from end succeeds -> N.

                    position = Obj::seek(fd, lengths[i], Obj::e_SEEK_FROM_END);
                    LOOP_ASSERT(position, numBytesA + lengths[i] == position);

                    position = Obj::seek(fd, -lengths[i],
                                                         Obj::e_SEEK_FROM_END);
                    LOOP_ASSERT(position, numBytesA - lengths[i] == position);

                    // Confirm that seek from begin succeeds -> N.

                    position = Obj::seek(fd, lengths[i],
                                                   Obj::e_SEEK_FROM_BEGINNING);
                    LOOP_ASSERT(position, lengths[i] == position);

                    // Confirm that seek from current succeeds -> N + M.

                    Obj::Offset origin = position;
                    for (int j = 0; j < lengths[i]; ++j) {
                        // Seek forward

                        position = Obj::seek(fd, j, Obj::e_SEEK_FROM_CURRENT);
                        LOOP_ASSERT(position, origin + j == position);

                        // Reset to 'lengths[i]'.

                        position = Obj::seek(fd, lengths[i],
                                                   Obj::e_SEEK_FROM_BEGINNING);
                        LOOP_ASSERT(position, lengths[i] == position);

                        // Seek backward

                        position = Obj::seek(fd, -j, Obj::e_SEEK_FROM_CURRENT);
                        LOOP_ASSERT(position, origin - j == position);

                        // Reset to 'lengths[i]'.

                        position = Obj::seek(fd, lengths[i],
                                                   Obj::e_SEEK_FROM_BEGINNING);
                        LOOP_ASSERT(position, lengths[i] == position);
                    }
                }

                Obj::close(fd);
            }

            if (veryVerbose) { cout << "\tSeek + read." << endl; }

            // Seek + read

            {
                // Prepare file with existing data.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }
                LOOP_ASSERT(fileName, !Obj::exists(fileName));

                Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                Obj::write(fd, blockA, numBytesA);

                Obj::close(fd);

                //   Re-open file and seek.

                fd = Obj::open(fileName, Obj::e_OPEN, Obj::e_READ_WRITE);
                ASSERT(Obj::k_INVALID_FD != fd);

                for (int i = 0; i < numLengths; ++i) {
                    // Confirm that read occurs at seek point, seeking to
                    // (0, 1, N).

                    Obj::Offset position = Obj::seek(fd, lengths[i],
                                                   Obj::e_SEEK_FROM_BEGINNING);
                    LOOP_ASSERT(position, lengths[i] == position);

                    memset(buffer, 0, sizeof(buffer));
                    int bytesRead = Obj::read(fd, buffer, 1);
                    LOOP2_ASSERT(lengths[i], bytesRead, 1 == bytesRead);
                    LOOP2_ASSERT(lengths[i], buffer[0], blockA[lengths[i]]);

                    // Confirm that read get 0 bytes if beyond end of file.

                    position = Obj::seek(fd, lengths[i], Obj::e_SEEK_FROM_END);
                    LOOP2_ASSERT(lengths[i], position,
                                           numBytesA + lengths[i] == position);

                    memset(buffer, 0, sizeof(buffer));
                    bytesRead = Obj::read(fd, buffer, 1);
                    LOOP2_ASSERT(lengths[i], bytesRead, 0 == bytesRead);

                    // Confirm that position after read is reported by "tell"

                    position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
                    LOOP2_ASSERT(lengths[i], position, 0 == position);

                    bytesRead = Obj::read(fd, buffer, lengths[i]);
                    LOOP2_ASSERT(lengths[i], bytesRead,
                                                      lengths[i] == bytesRead);

                    position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT);
                    LOOP2_ASSERT(lengths[i], position, lengths[i] == position);
                }

                Obj::close(fd);
            }


            if (veryVerbose) { cout << "\tSeek + write." << endl; }

            // Seek + write

            {
                for (int i = 0; i < numLengths; ++i) {
                    // Prepare file> with existing data.

                    if (Obj::exists(fileName)) {
                        Obj::remove(fileName);
                    }
                    LOOP_ASSERT(fileName, !Obj::exists(fileName));

                    Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                    ASSERT(Obj::k_INVALID_FD != fd);

                    Obj::write(fd, blockA, numBytesA);

                    // Confirm that write occurs at seek point, seeking to
                    // (0, 1, N).

                    Obj::Offset position = Obj::seek(fd, lengths[i],
                                                   Obj::e_SEEK_FROM_BEGINNING);
                    LOOP_ASSERT(position, lengths[i] == position);

                    int bytesWritten = Obj::write(fd, blockB, 1);
                    LOOP2_ASSERT(lengths[i], bytesWritten, 1 == bytesWritten);

                    position = Obj::seek(fd, lengths[i],
                                                   Obj::e_SEEK_FROM_BEGINNING);
                    LOOP_ASSERT(position, lengths[i] == position);

                    memset(buffer, 0, sizeof(buffer));
                    int bytesRead = Obj::read(fd, buffer, 1);
                    LOOP2_ASSERT(lengths[i], bytesRead, 1 == bytesRead);
                    LOOP2_ASSERT(lengths[i], buffer[0], blockB[0]);

                    // Reset overwritten content

                    position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
                    LOOP_ASSERT(position, 0 == position);

                    Obj::write(fd, blockA, numBytesA);

                    position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);

                    memset(buffer, 0, sizeof(buffer));
                    bytesRead = Obj::read(fd, buffer, numBytesA);
                    LOOP2_ASSERT(lengths[i], bytesRead,
                                                       numBytesA == bytesRead);
                    LOOP_ASSERT(lengths[i], 0 == memcmp(buffer, blockA,
                                                                   numBytesA));

                    // Confirm that write fails if seek point is beyond end of
                    // file.

                    position = Obj::seek(fd, lengths[i], Obj::e_SEEK_FROM_END);
                    LOOP2_ASSERT(lengths[i], position,
                                           numBytesA + lengths[i] == position);

                    bytesWritten = Obj::write(fd, blockB, 1);
                    LOOP2_ASSERT(lengths[i], bytesWritten, 1 == bytesWritten);

                    position = Obj::seek( fd, numBytesA + lengths[i],
                                                   Obj::e_SEEK_FROM_BEGINNING);
                    LOOP_ASSERT(position, numBytesA + lengths[i] == position);

                    memset(buffer, 0, sizeof(buffer));
                    bytesRead = Obj::read(fd, buffer, 1);
                    LOOP2_ASSERT(lengths[i], bytesRead, 1 == bytesRead);
                    LOOP2_ASSERT(lengths[i], buffer[0], blockB[0]);

                    // Confirm that position after write is reported by "tell"

                    position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING);
                    LOOP2_ASSERT(lengths[i], position, 0 == position);

                    bytesWritten = Obj::write(fd, blockB, lengths[i]);
                    LOOP2_ASSERT(lengths[i], bytesWritten,
                                                   lengths[i] == bytesWritten);

                    position = Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT);
                    LOOP2_ASSERT(lengths[i], position, lengths[i] == position);

                    Obj::close(fd);
                    Obj::remove(fileName);
                }
            }
        }

        if (verbose) cout <<
                        "\nBootstrap, phase 5: 'open' with truncation."
                          << endl;
        {
            const Obj::FileOpenPolicy OP = Obj::e_OPEN;
            const Obj::FileOpenPolicy CR = Obj::e_CREATE;
            const Obj::FileOpenPolicy OC = Obj::e_OPEN_OR_CREATE;

            const Obj::FileIOPolicy RO = Obj::e_READ_ONLY;
            const Obj::FileIOPolicy WO = Obj::e_WRITE_ONLY;
            const Obj::FileIOPolicy AO = Obj::e_APPEND_ONLY;
            const Obj::FileIOPolicy RW = Obj::e_READ_WRITE;
            const Obj::FileIOPolicy RA = Obj::e_READ_APPEND;

            const Obj::FileTruncatePolicy TR = Obj::e_TRUNCATE;
            const Obj::FileTruncatePolicy KP = Obj::e_KEEP;

            const struct DataRow {
                int d_line;
                Obj::FileOpenPolicy d_openPolicy;
                Obj::FileIOPolicy d_ioPolicy;
                Obj::FileTruncatePolicy d_initPolicy;
                bool d_exists;
            } DATA[] = {
                //LN openPolicy ioPolicy initPolicy exists
                //-- ---------- -------- ---------- ------
                {L_, CR,        WO,      KP,       false  },
                {L_, OP,        WO,      KP,        true  },
                {L_, OC,        WO,      KP,       false  },
                {L_, OC,        WO,      KP,        true  },

                {L_, CR,        RO,      KP,       false  },
                {L_, OP,        RO,      KP,        true  },
                {L_, OC,        RO,      KP,       false  },
                {L_, OC,        RO,      KP,        true  },

                {L_, CR,        AO,      KP,       false  },
                {L_, OP,        AO,      KP,        true  },
                {L_, OC,        AO,      KP,       false  },
                {L_, OC,        AO,      KP,        true  },

                {L_, CR,        RW,      KP,       false  },
                {L_, OP,        RW,      KP,        true  },
                {L_, OC,        RW,      KP,       false  },
                {L_, OC,        RW,      KP,        true  },

                {L_, CR,        RA,      KP,       false  },
                {L_, OP,        RA,      KP,        true  },
                {L_, OC,        RA,      KP,       false  },
                {L_, OC,        RA,      KP,        true  },

                {L_, CR,        WO,      TR,       false  },
                {L_, OP,        WO,      TR,        true  },
                {L_, OC,        WO,      TR,       false  },
                {L_, OC,        WO,      TR,        true  },

                {L_, CR,        RO,      TR,       false  },
                // Truncate without Write or Create disallowed:
                // {L_, OP,        RO,      TR,        true  },
                {L_, OC,        RO,      TR,       false  },
                {L_, OC,        RO,      TR,        true  },

                {L_, CR,        AO,      TR,       false  },
                // Truncate without Write or Create disallowed:
                // {L_, OP,        AO,      TR,        true  },
                {L_, OC,        AO,      TR,       false  },
                {L_, OC,        AO,      TR,        true  },

                {L_, CR,        RW,      TR,       false  },
                {L_, OP,        RW,      TR,        true  },
                {L_, OC,        RW,      TR,       false  },
                {L_, OC,        RW,      TR,        true  },

                {L_, CR,        RA,      TR,       false  },
                // Truncate without Write or Create disallowed:
                // {L_, OP,        RA,      TR,        true  },
                {L_, OC,        RA,      TR,       false  },
                {L_, OC,        RA,      TR,        true  },
            };
            int NUM_DATA = sizeof(DATA) / sizeof(DataRow);

            bsl::string fileName(tempFileName());

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int                       LINE   = DATA[ti].d_line;
                const bool                      EXISTS = DATA[ti].d_exists;

                const Obj::FileOpenPolicy     OPEN_POLICY
                                                       = DATA[ti].d_openPolicy;
                const Obj::FileIOPolicy       IO_POLICY
                                                       = DATA[ti].d_ioPolicy;
                const Obj::FileTruncatePolicy INIT_POLICY
                                                       = DATA[ti].d_initPolicy;

                if (veryVerbose) {
                    T_()
                    P_(OPEN_POLICY) P_(IO_POLICY) P_(INIT_POLICY) P(EXISTS)
                }

                // Establish a consistent baseline: 'fileName' does not exist.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }

                LOOP2_ASSERT(LINE, fileName, !Obj::exists(fileName));

                // If this test expects 'fileName' to exist, it must be created
                // using the (already-tested) 'e_CREATE' open mode.

                if (EXISTS) {
                    Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                    LOOP_ASSERT(LINE, Obj::k_INVALID_FD != fd);
                    LOOP_ASSERT(LINE, 1 == Obj::write(fd, " ", 1));

                    Obj::close(fd);

                    LOOP2_ASSERT(LINE, fileName, Obj::exists(fileName));
                    LOOP_ASSERT(LINE, 1 == Obj::getFileSize(fileName));
                }

                Obj::FileDescriptor fd = Obj::open(
                                fileName, OPEN_POLICY, IO_POLICY, INIT_POLICY);
                LOOP2_ASSERT(LINE, fd, Obj::k_INVALID_FD != fd);

                if (Obj::k_INVALID_FD == fd) {
                    // We can do no more testing with an invalid file
                    // descriptor, so go on to the next test.

                    continue;
                }

                // Test truncation

                if (!EXISTS || Obj::e_TRUNCATE == INIT_POLICY) {
                    LOOP_ASSERT(LINE, 0 ==
                                       Obj::seek(fd, 0, Obj::e_SEEK_FROM_END));
                }
                else if (Obj::e_KEEP == INIT_POLICY) {
                    LOOP_ASSERT(LINE, 0 !=
                                       Obj::seek(fd, 0, Obj::e_SEEK_FROM_END));
                }
                else {
                    LOOP_ASSERT(LINE, false);
                }

                Obj::close(fd);
                Obj::remove(fileName);
            }
        }

        if (verbose) cout <<
                            "\nBootstrap, phase 6: 'open' with append."
                          << endl;
        {
            const Obj::FileOpenPolicy OP = Obj::e_OPEN;
            const Obj::FileOpenPolicy CR = Obj::e_CREATE;
            const Obj::FileOpenPolicy OC = Obj::e_OPEN_OR_CREATE;

            const Obj::FileIOPolicy RO = Obj::e_READ_ONLY;
            const Obj::FileIOPolicy WO = Obj::e_WRITE_ONLY;
            const Obj::FileIOPolicy AO = Obj::e_APPEND_ONLY;
            const Obj::FileIOPolicy RW = Obj::e_READ_WRITE;
            const Obj::FileIOPolicy RA = Obj::e_READ_APPEND;

            const Obj::FileTruncatePolicy TR = Obj::e_TRUNCATE;
            const Obj::FileTruncatePolicy KP = Obj::e_KEEP;

            const struct DataRow {
                int d_line;
                Obj::FileOpenPolicy d_openPolicy;
                Obj::FileIOPolicy d_ioPolicy;
                Obj::FileTruncatePolicy d_initPolicy;
                bool d_exists;
            } DATA[] = {
                //LN openPolicy ioPolicy initPolicy exists
                //-- ---------- -------- ---------- ------
                {L_, CR,        WO,      KP,       false  },
                {L_, OP,        WO,      KP,        true  },
                {L_, OC,        WO,      KP,       false  },
                {L_, OC,        WO,      KP,        true  },

                {L_, CR,        RO,      KP,       false  },
                {L_, OP,        RO,      KP,        true  },
                {L_, OC,        RO,      KP,       false  },
                {L_, OC,        RO,      KP,        true  },

                {L_, CR,        AO,      KP,       false  },
                {L_, OP,        AO,      KP,        true  },
                {L_, OC,        AO,      KP,       false  },
                {L_, OC,        AO,      KP,        true  },

                {L_, CR,        RW,      KP,       false  },
                {L_, OP,        RW,      KP,        true  },
                {L_, OC,        RW,      KP,       false  },
                {L_, OC,        RW,      KP,        true  },

                {L_, CR,        RA,      KP,       false  },
                {L_, OP,        RA,      KP,        true  },
                {L_, OC,        RA,      KP,       false  },
                {L_, OC,        RA,      KP,        true  },

                {L_, CR,        WO,      TR,       false  },
                {L_, OP,        WO,      TR,        true  },
                {L_, OC,        WO,      TR,       false  },
                {L_, OC,        WO,      TR,        true  },

                {L_, CR,        RO,      TR,       false  },
                // Truncate without Write or Create disallowed:
                // {L_, OP,        RO,      TR,        true  },
                {L_, OC,        RO,      TR,       false  },
                {L_, OC,        RO,      TR,        true  },

                {L_, CR,        AO,      TR,       false  },
                // Truncate without Write or Create disallowed:
                // {L_, OP,        AO,      TR,        true  },
                {L_, OC,        AO,      TR,       false  },
                {L_, OC,        AO,      TR,        true  },

                {L_, CR,        RW,      TR,       false  },
                {L_, OP,        RW,      TR,        true  },
                {L_, OC,        RW,      TR,       false  },
                {L_, OC,        RW,      TR,        true  },

                {L_, CR,        RA,      TR,       false  },
                // Truncate without Write or Create disallowed:
                // {L_, OP,        RA,      TR,        true  },
                {L_, OC,        RA,      TR,       false  },
                {L_, OC,        RA,      TR,        true  },
            };
            int NUM_DATA = sizeof(DATA) / sizeof(DataRow);

            const char blockA[]  = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
            const int  numBytesA = sizeof(blockA);
            const char blockB[]  = { '0', '1', '2', '3', '4' };
            const int  numBytesB = sizeof(blockB);

            bsl::string fileName(tempFileName());

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int                       LINE   = DATA[ti].d_line;
                const bool                      EXISTS = DATA[ti].d_exists;

                const Obj::FileOpenPolicy     OPEN_POLICY
                                                       = DATA[ti].d_openPolicy;
                const Obj::FileIOPolicy       IO_POLICY
                                                       = DATA[ti].d_ioPolicy;
                const Obj::FileTruncatePolicy INIT_POLICY
                                                       = DATA[ti].d_initPolicy;

                if (veryVerbose) {
                    T_()
                    P_(OPEN_POLICY) P_(IO_POLICY) P_(INIT_POLICY) P(EXISTS)
                }

                // Establish a consistent baseline: 'fileName' does not exist.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }

                LOOP2_ASSERT(LINE, fileName, !Obj::exists(fileName));

                // If this test expects 'fileName' to exist, it must be created
                // using the (already-tested) 'e_CREATE' open mode.

                if (EXISTS) {
                    Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_READ_WRITE);
                    LOOP_ASSERT(LINE, Obj::k_INVALID_FD != fd);
                    LOOP_ASSERT(LINE, 1 == Obj::write(fd, " ", 1));

                    Obj::close(fd);

                    LOOP2_ASSERT(LINE, fileName, Obj::exists(fileName));
                    LOOP_ASSERT(LINE, 1 == Obj::getFileSize(fileName));
                }

                Obj::FileDescriptor fd = Obj::open(
                                fileName, OPEN_POLICY, IO_POLICY, INIT_POLICY);
                LOOP2_ASSERT(LINE, fd, Obj::k_INVALID_FD != fd);

                if (Obj::k_INVALID_FD == fd) {
                    // We can do no more testing with an invalid file
                    // descriptor, so go on to the next test.

                    continue;
                }

                // Test append

                if (Obj::e_WRITE_ONLY  == IO_POLICY ||
                    Obj::e_READ_WRITE  == IO_POLICY) {
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesA ==
                                            Obj::write(fd, blockA, numBytesA));
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesB ==
                                            Obj::write(fd, blockB, numBytesB));

                    LOOP_ASSERT(LINE, numBytesB ==
                                   Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT));
                }
                else if (Obj::e_APPEND_ONLY == IO_POLICY ||
                         Obj::e_READ_APPEND == IO_POLICY) {
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesA ==
                                            Obj::write(fd, blockA, numBytesA));
                    Obj::Offset endpoint =
                                        Obj::seek(fd, 0, Obj::e_SEEK_FROM_END);
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesB ==
                            Obj::write(fd, blockB, numBytesB));

                    LOOP_ASSERT(LINE, endpoint + numBytesB ==
                                   Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT));
                }
                else {
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, 0 > Obj::write(fd, blockA, numBytesA));
                }

                Obj::close(fd);
                Obj::remove(fileName);
            }

        }

        if (verbose) cout << "\nFull test of enum-based four-argument open"
                          << endl;
        {
            const Obj::FileOpenPolicy OP = Obj::e_OPEN;
            const Obj::FileOpenPolicy CR = Obj::e_CREATE;
            const Obj::FileOpenPolicy OC = Obj::e_OPEN_OR_CREATE;

            const Obj::FileIOPolicy RO = Obj::e_READ_ONLY;
            const Obj::FileIOPolicy WO = Obj::e_WRITE_ONLY;
            const Obj::FileIOPolicy AO = Obj::e_APPEND_ONLY;
            const Obj::FileIOPolicy RW = Obj::e_READ_WRITE;
            const Obj::FileIOPolicy RA = Obj::e_READ_APPEND;

            const Obj::FileTruncatePolicy TR = Obj::e_TRUNCATE;
            const Obj::FileTruncatePolicy KP = Obj::e_KEEP;

            const struct DataRow {
                int d_line;
                Obj::FileOpenPolicy d_openPolicy;
                Obj::FileIOPolicy d_ioPolicy;
                Obj::FileTruncatePolicy d_initPolicy;
                bool d_exists;
                bool d_success;
            } DATA[] = {
                //LN openPolicy ioPolicy initPolicy exists success
                //-- ---------- -------- ---------- ------ -------
                {L_, CR,        WO,      KP,       false,   true },
                {L_, CR,        WO,      KP,        true,  false },
                {L_, OP,        WO,      KP,       false,  false },
                {L_, OP,        WO,      KP,        true,   true },
                {L_, OC,        WO,      KP,       false,   true },
                {L_, OC,        WO,      KP,        true,   true },

                {L_, CR,        RO,      KP,       false,   true },
                {L_, CR,        RO,      KP,        true,  false },
                {L_, OP,        RO,      KP,       false,  false },
                {L_, OP,        RO,      KP,        true,   true },
                {L_, OC,        RO,      KP,       false,   true },
                {L_, OC,        RO,      KP,        true,   true },

                {L_, CR,        AO,      KP,       false,   true },
                {L_, CR,        AO,      KP,        true,  false },
                {L_, OP,        AO,      KP,       false,  false },
                {L_, OP,        AO,      KP,        true,   true },
                {L_, OC,        AO,      KP,       false,   true },
                {L_, OC,        AO,      KP,        true,   true },

                {L_, CR,        RW,      KP,       false,   true },
                {L_, CR,        RW,      KP,        true,  false },
                {L_, OP,        RW,      KP,       false,  false },
                {L_, OP,        RW,      KP,        true,   true },
                {L_, OC,        RW,      KP,       false,   true },
                {L_, OC,        RW,      KP,        true,   true },

                {L_, CR,        RA,      KP,       false,   true },
                {L_, CR,        RA,      KP,        true,  false },
                {L_, OP,        RA,      KP,       false,  false },
                {L_, OP,        RA,      KP,        true,   true },
                {L_, OC,        RA,      KP,       false,   true },
                {L_, OC,        RA,      KP,        true,   true },

                {L_, CR,        WO,      TR,       false,   true },
                {L_, CR,        WO,      TR,        true,  false },
                {L_, OP,        WO,      TR,       false,  false },
                {L_, OP,        WO,      TR,        true,   true },
                {L_, OC,        WO,      TR,       false,   true },
                {L_, OC,        WO,      TR,        true,   true },

                {L_, CR,        RO,      TR,       false,   true },
                {L_, CR,        RO,      TR,        true,  false },
                {L_, OP,        RO,      TR,       false,  false },
                // Truncate without Write or Create disallowed:
                {L_, OP,        RO,      TR,        true,  false },
                {L_, OC,        RO,      TR,       false,   true },
                {L_, OC,        RO,      TR,        true,   true },

                {L_, CR,        AO,      TR,       false,   true },
                {L_, CR,        AO,      TR,        true,  false },
                {L_, OP,        AO,      TR,       false,  false },
                // Truncate without Write or Create disallowed:
                {L_, OP,        AO,      TR,        true,  false },
                {L_, OC,        AO,      TR,       false,   true },
                {L_, OC,        AO,      TR,        true,   true },

                {L_, CR,        RW,      TR,       false,   true },
                {L_, CR,        RW,      TR,        true,  false },
                {L_, OP,        RW,      TR,       false,  false },
                {L_, OP,        RW,      TR,        true,   true },
                {L_, OC,        RW,      TR,       false,   true },
                {L_, OC,        RW,      TR,        true,   true },

                {L_, CR,        RA,      TR,       false,   true },
                {L_, CR,        RA,      TR,        true,  false },
                {L_, OP,        RA,      TR,       false,  false },
                // Truncate without Write or Create disallowed:
                {L_, OP,        RA,      TR,        true,  false },
                {L_, OC,        RA,      TR,       false,   true },
                {L_, OC,        RA,      TR,        true,   true },
            };
            int NUM_DATA = sizeof(DATA) / sizeof(DataRow);

            const char blockA[]  = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
            const int  numBytesA = sizeof(blockA);
            const char blockB[]  = { '0', '1', '2', '3', '4' };
            const int  numBytesB = sizeof(blockB);
            char       buffer[8];

            bool isCreateFileTested = false;

            bsl::string fileName(tempFileName());

            for (int ti = 0; ti < NUM_DATA; ++ti) {
                const int                  LINE        = DATA[ti].d_line;
                const bool                 EXISTS      = DATA[ti].d_exists;
                const bool                 EXP_SUCCESS = DATA[ti].d_success;

                const Obj::FileOpenPolicy     OPEN_POLICY
                                                       = DATA[ti].d_openPolicy;
                const Obj::FileIOPolicy       IO_POLICY
                                                       = DATA[ti].d_ioPolicy;
                const Obj::FileTruncatePolicy INIT_POLICY
                                                       = DATA[ti].d_initPolicy;

                if (verbose) {
                    T_()
                    P_(OPEN_POLICY) P_(IO_POLICY) P_(INIT_POLICY) P(EXISTS)
                }

                // Establish a consistent baseline: 'fileName' does not exist.

                if (Obj::exists(fileName)) {
                    Obj::remove(fileName);
                }

                LOOP2_ASSERT(LINE, fileName, !Obj::exists(fileName));

                // If this test expects 'fileName' to exist, it must be created
                // using the (already-tested) 'e_CREATE' open mode.

                if (EXISTS) {
                    // We cannot use the combination of 'e_CREATE' with
                    // 'e_WRITE_ONLY' until it has been tested.

                    LOOP_ASSERT(LINE, isCreateFileTested);

                    Obj::FileDescriptor fd = Obj::open(
                                   fileName, Obj::e_CREATE, Obj::e_WRITE_ONLY);
                    LOOP_ASSERT(LINE, Obj::k_INVALID_FD != fd);
                    LOOP_ASSERT(LINE, 1 == Obj::write(fd, " ", 1));

                    Obj::close(fd);

                    LOOP2_ASSERT(LINE, fileName, Obj::exists(fileName));
                    LOOP_ASSERT(LINE, 1 == Obj::getFileSize(fileName));
                }

                // Test open success

                Obj::FileDescriptor fd = Obj::open(
                                fileName, OPEN_POLICY, IO_POLICY, INIT_POLICY);
                LOOP3_ASSERT(LINE, EXP_SUCCESS, fd,
                                     EXP_SUCCESS == (Obj::k_INVALID_FD != fd));

                if (Obj::k_INVALID_FD == fd) {
                    // Expected failure.  We can do no more testing with an
                    // invalid file descriptor, so go on to the next test.

                    continue;
                }

                // We expect to start at offset 0

                LOOP_ASSERT(LINE, 0 == Obj::seek(fd, 0,
                                                    Obj::e_SEEK_FROM_CURRENT));

                // Test truncation

                if (!EXISTS || Obj::e_TRUNCATE == INIT_POLICY) {
                    LOOP_ASSERT(LINE, 0 ==
                                       Obj::seek(fd, 0, Obj::e_SEEK_FROM_END));
                }
                else if (Obj::e_KEEP == INIT_POLICY) {
                    LOOP_ASSERT(LINE, 0 !=
                                       Obj::seek(fd, 0, Obj::e_SEEK_FROM_END));
                }
                else {
                    LOOP_ASSERT(LINE, false);
                }

                // Test writing

                if (Obj::e_WRITE_ONLY  == IO_POLICY ||
                    Obj::e_READ_WRITE  == IO_POLICY) {
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesA ==
                                            Obj::write(fd, blockA, numBytesA));
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesB ==
                                            Obj::write(fd, blockB, numBytesB));

                    LOOP_ASSERT(LINE, numBytesB ==
                                   Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT));
                }
                else if (Obj::e_APPEND_ONLY == IO_POLICY ||
                         Obj::e_READ_APPEND == IO_POLICY) {
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesA ==
                                            Obj::write(fd, blockA, numBytesA));
                    Obj::Offset endpoint =
                                        Obj::seek(fd, 0, Obj::e_SEEK_FROM_END);
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, numBytesB ==
                                            Obj::write(fd, blockB, numBytesB));

                    LOOP_ASSERT(LINE, endpoint + numBytesB ==
                                   Obj::seek(fd, 0, Obj::e_SEEK_FROM_CURRENT));
                }
                else {
                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, 0 > Obj::write(fd, blockA, numBytesA));
                }

                // Test reading

                if (   Obj::e_READ_ONLY   == IO_POLICY
                    || Obj::e_READ_WRITE  == IO_POLICY
                    || Obj::e_READ_APPEND == IO_POLICY) {
                    // Seek to prepared read location.

                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, 0 <= Obj::read(fd, buffer, 1));
                }
                else {
                    // Seek to prepared read location.

                    LOOP_ASSERT(LINE, 0 ==
                                 Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
                    LOOP_ASSERT(LINE, 0 > Obj::read(fd, buffer, 1));
                }

                if (   Obj::e_CREATE     == OPEN_POLICY
                    && Obj::e_WRITE_ONLY == IO_POLICY) {
                    isCreateFileTested = true;
                }

                Obj::close(fd);
                Obj::remove(fileName);
            }
        }

        if (verbose) cout << "\nNegative testing" << endl;
        {
            bsl::string fileName(tempFileName());

            if (Obj::exists(fileName)) {
                Obj::remove(fileName);
            }
            LOOP_ASSERT(fileName, !Obj::exists(fileName));

            bsls::AssertFailureHandlerGuard hG(
                                             bsls::AssertTest::failTestDriver);

            {
                Obj::FileDescriptor fd = Obj::k_INVALID_FD;

                ASSERT_PASS(fd = Obj::open(fileName.c_str(),
                                           Obj::e_OPEN_OR_CREATE,
                                           Obj::e_READ_ONLY));

                if (fd != Obj::k_INVALID_FD) {
                    Obj::close(fd);
                }
            }

            {
                Obj::FileDescriptor fd = Obj::k_INVALID_FD;

                ASSERT_FAIL(fd = Obj::open(0,
                                           Obj::e_OPEN_OR_CREATE,
                                           Obj::e_READ_ONLY));

                ASSERT(Obj::k_INVALID_FD == fd);
                if (fd != Obj::k_INVALID_FD) {
                    Obj::close(fd);
                }
            }

            if (Obj::exists(fileName)) {
                Obj::remove(fileName);
            }
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // Usage-Example-Like Functionality Test
        //
        // Concern: The *functionality* of the component's usage example
        //          is correct.
        //
        // Reason for existence: We want to ensure that the usage example
        //                       as written will compile and execute safely,
        //                       which makes *validating* it awkward: either
        //                       the usage example bloats with setup,
        //                       validation, and cleanup; or the usage example
        //                       test includes a lot of code not in the usage
        //                       example.
        //
        // Plan: copy the usage example test.  Before running, set up
        //       files in the source directory and, if possible, make some
        //       of them old.  (On POSIX, utime() is available to do this; but
        //       SetFileInformationByHandle(), the equivalent Win32 function,
        //       is only available in Vista and later.)  Then after the usage
        //       example executes, verify the number of files in each directory
        //       matches what's expected.
        //
        //       This will also serve as a basic test of findMatchingPaths(),
        //       open(), and write().
        // --------------------------------------------------------------------

        if (verbose) cout << "\nUsage Example like Testing"
                          << "\n==========================" << endl;


        for (size_t ni = 0; ni < NUM_VALID_NAMES; ++ni) {
            const char *const NAME = NAMES[ni];

            if (veryVerbose) { T_() P_(ni) P(NAME) }

            bsl::wstring wide;
            int          rc = bdede_CharConvertUtf16::utf8ToUtf16(&wide, NAME);

            LOOP2_ASSERT(ni, rc, ni == NAME_ANSI ? rc != 0 : rc == 0);

            bsl::string logPath = NAME;

            Obj::remove(logPath.c_str(), true);
            bdesu_PathUtil::appendRaw(&logPath, "logs");

            bsl::string oldPath(logPath), newPath(logPath);
            bdesu_PathUtil::appendRaw(&oldPath, "old");
            bdesu_PathUtil::appendRaw(&newPath, "new");
            LOOP_ASSERT(oldPath,
                           0 == Obj::createDirectories(oldPath.c_str(), true));
            LOOP_ASSERT(newPath,
                           0 == Obj::createDirectories(newPath.c_str(), true));

            ASSERT(Obj::exists(oldPath));
            ASSERT(Obj::exists(newPath));

            enum {
                k_FILENAME_BUFFER_SIZE = 32
            };

            char filenameBuffer[k_FILENAME_BUFFER_SIZE];

            // TBD: When SetFileInformationByHandle() is available, then we
            // should write a setModificationTime() method and use it here (see
            // bdesu_filesystemutil.h).  Until then, we use utime() on POSIX
            // directly and we do not attempt to "touch" Windows files.

            enum {
                NUM_TOTAL_FILES = 10,
#ifdef BSLS_PLATFORM_OS_WINDOWS
                NUM_OLD_FILES = 0,
#else
                NUM_OLD_FILES = 3,
#endif
                NUM_NEW_FILES = NUM_TOTAL_FILES - NUM_OLD_FILES
            };

            for (int i = 0; i < NUM_TOTAL_FILES; ++i) {
                bool isOld = i < NUM_OLD_FILES;

                int filenameLength = sprintf(filenameBuffer,
                                             "filesystemutil%02d_%c.log", i,
                                             isOld ? 'o' : 'n');

                ASSERT(0 == bdesu_PathUtil::appendIfValid(&logPath,
                            filenameBuffer));

                if (Obj::exists(logPath.c_str())) {
                    ASSERT(0 == Obj::remove(logPath.c_str(), true));
                }
                Obj::FileDescriptor f = Obj::open(logPath.c_str(),
                                                  Obj::e_OPEN_OR_CREATE,
                                                  Obj::e_READ_WRITE);
                LOOP_ASSERT(logPath, f != Obj::k_INVALID_FD);

                filenameBuffer[filenameLength++] = '\n';
                filenameBuffer[filenameLength++] = 0;

                ASSERT(filenameLength == Obj::write(f,
                                                    filenameBuffer,
                                                    filenameLength));
                ASSERT(0 == Obj::close(f));

#ifndef BSLS_PLATFORM_OS_WINDOWS
                bdet_TimeInterval threeDaysAgo = bdetu_SystemTime::now() -
                    bdet_TimeInterval(3 * 24 * 3600, 0);
                if (isOld) {
                    struct utimbuf timeInfo;
                    timeInfo.actime = timeInfo.modtime =
                        (bsl::time_t)threeDaysAgo.seconds();

                    //test invariant:

                    ASSERT(0 == utime(logPath.c_str(), &timeInfo));
                }
#endif
                bdesu_PathUtil::popLeaf(&logPath);
            }

            bdesu_PathUtil::appendRaw(&logPath, "*.log");
            vector<bsl::string> logFiles;
            Obj::findMatchingPaths(&logFiles, logPath.c_str());
            bdesu_PathUtil::popLeaf(&logPath);

            bdet_Datetime modTime;
            string        fileName;
            bdet_Datetime nowTime = bdetu_SystemTime::nowAsDatetimeUtc();
            for (vector<bsl::string>::iterator it = logFiles.begin();
                    it != logFiles.end(); ++it) {
                ASSERT(0 == Obj::getLastModificationTime(&modTime,
                            it->c_str()));
                bdesu_PathUtil::getLeaf(&fileName, *it);
                bsl::string* whichDirectory =
                     2 < (nowTime - modTime).totalDays() ? &oldPath : &newPath;
                bdesu_PathUtil::appendRaw(whichDirectory, fileName.c_str());
                ASSERT(0 == Obj::move(it->c_str(),
                            whichDirectory->c_str()));
                bdesu_PathUtil::popLeaf(whichDirectory);
            }

            // Now validate

            bdesu_PathUtil::appendRaw(&logPath, "*");
            bdesu_PathUtil::appendRaw(&logPath, "*o*.log");
            Obj::findMatchingPaths(&logFiles, logPath.c_str());
            ASSERT(NUM_OLD_FILES == logFiles.size());
            bdesu_PathUtil::popLeaf(&logPath);

            bdesu_PathUtil::appendRaw(&logPath, "*n*.log");
            Obj::findMatchingPaths(&logFiles, logPath.c_str());
            ASSERT(NUM_NEW_FILES == logFiles.size());
            bdesu_PathUtil::popLeaf(&logPath);
            bdesu_PathUtil::popLeaf(&logPath);

            // Clean up

            ASSERT(0 == bdesu_PathUtil::popLeaf(&logPath));
            ASSERT(0 == Obj::remove(logPath.c_str(), true));
        }
      } break;
      case -1: {
        // --------------------------------------------------------------------
        // TESTING detection of large files
        //
        // Concern: Whether 'getFileSize' can detect a large file (> 4GB) since
        //          the file size becomes a 64-bit number.
        //
        // Plan: Create a large file in "/tmp" and check the file size.
        //       Remove it afterwards.
        //
        // --------------------------------------------------------------------

#ifndef BSLS_PLATFORM_OS_WINDOWS
        // Not sure how to create large files on windows, so test only on UNIX.
        // However, this function had been tested on windows by creating the
        // large file through cygwin.

        // Create a 5 GB file.

        if (veryVerbose) cout << "\n3. Large File" << endl;

        system("dd if=/dev/zero of=/tmp/fiveGBFile "
               "bs=1024000 count=5000");

        string fileName("/tmp/fiveGBFile");

        Obj::Offset off = Obj::getFileSize(fileName);
        ASSERT(5120000000LL == off);

        Obj::Offset off2 = Obj::getFileSize(fileName.c_str());
        ASSERT(5120000000LL == off2);

        if (veryVerbose) {
            cout << "Expected 5120000000LL" << endl;
            cout << "Actual ";
            P_(off) P(off2)
        }

        Obj::remove(fileName);
#endif
      } break;
      case -2: {
        // --------------------------------------------------------------------
        // --------------------------------------------------------------------

        static const char* foo = "/tmp/blahblah.tmp";
        Obj::remove(foo);
        Obj::FileDescriptor fd = Obj::open(foo,
                                           Obj::e_OPEN_OR_CREATE,
                                           Obj::e_READ_WRITE);
        int pageSize = bdesu_MemoryUtil::pageSize();
        Obj::growFile(fd, pageSize);
        int *p;
        ASSERT(0 == Obj::map(fd, (void**)&p, 0, pageSize,
                                   bdesu_MemoryUtil::BDESU_ACCESS_READ_WRITE));
        printf("mapped at %p\n", p);
        for (int i = 0; i < 10000; ++i) {
          ASSERT(0 == Obj::seek(fd, 0, Obj::e_SEEK_FROM_BEGINNING));
          int buf;
          *p = i;
          ASSERT(sizeof(int) == Obj::read(fd, &buf, sizeof(int)));
          ASSERT(i == buf);
        }
        ASSERT(0 == Obj::unmap(p, pageSize));

#if 0
        Obj::FileDescriptor fd =  // /bb/data is Bloomberg-specific
                       Obj::open("/bb/data/tmp/blahblah.tmp",
                                 Obj::e_OPEN_OR_CREATE,
                                 Obj::e_READ_WRITE);
        int pageSize = Obj::pageSize();
        int nPages = 1024*1024*1024 / pageSize;
        int rc = Obj::growFile(fd, nPages * pageSize, argc > 2);
        printf("growFile(fd, %d, %d) rc=%d\n",
               nPages * pageSize, (int)(argc>2), rc);
        int fileSize = Obj::seek(fd, 0, SEEK_END);
        printf("file size = %d\n", fileSize);
        if (!rc) {
            for(int i=0; i<nPages; i++) {
                bdesu_FilesystemUtilMapping fm =
                                    Obj::map(fd, i * pageSize, pageSize, true);
                memset(fm.addr(), 2, pageSize);
                Obj::unmap(fm, pageSize);
            }
        }
#endif
      } break;
      case -3: {
        // --------------------------------------------------------------------
        // LARGE FILE TEST CASE
        //
        // Concern:
        //   We need a straightforward test case, using writes and reads,
        //   to create and read back a 5G file.
        // --------------------------------------------------------------------

        if (verbose) cout << "SIMPLE 5G FILE TEST CASE\n"
                             "========================\n";

        typedef bdesu_FilesystemUtil Util;

#if 1
        const bsls::Types::Int64 fiveGig = 5LL * 1000LL * 1000LL * 1000LL;
        const bsls::Types::Int64 deltaMileStone = 100LL * 1000LL * 1000LL;
#else
        const bsls::Types::Int64 fiveGig = 5 * 1000LL * 1000LL;
        const bsls::Types::Int64 deltaMileStone = 100LL * 1000LL;
#endif

        bsls::Types::Int64 mileStone = deltaMileStone;

        bsls::Types::Int64 bytesWritten = 0;

        char record[80] = "123456789 123456789 123456789 123456789 "
                          "123456789 123456789 123";
        char * const writeTo = record + 63;

        MMIXRand rand;

        LOOP_ASSERT(Util::getFileSizeLimit(),
                                           Util::getFileSizeLimit() > fiveGig);

        const char *fileName = "tmpFiveGig.txt";
        Util::FileDescriptor fd = Util::open(fileName,
                                             Obj::e_OPEN_OR_CREATE,
                                             Obj::e_READ_WRITE);
        ASSERT(Util::k_INVALID_FD != fd);

        for (;;) {
            rand.munge();
            bsl::strcpy(writeTo, rand.display());
            record[79] = '\n';

            int rc = Util::write(fd, record, 80);
            if (80 != rc) {
                ASSERT(0 && "80 != rc");
                break;
            }
            bytesWritten += 80;

            if (bytesWritten >= mileStone) {
                cout << bytesWritten << " written -- last: " <<
                                                        rand.display() << endl;
                if (bytesWritten >= fiveGig) {
                    break;
                }
                mileStone += deltaMileStone;
            }
        }
        ASSERT(fiveGig == bytesWritten);
        ASSERT(Util::seek(fd, 0, Util::e_SEEK_FROM_CURRENT) == bytesWritten);
        ASSERT(Util::seek(fd, 0, Util::e_SEEK_FROM_END)     == bytesWritten);

        cout << "Writing done\n";

        if (verbose) P(bytesWritten);

        ASSERT(Util::getFileSize(fileName) == bytesWritten);
        ASSERT(Util::isRegularFile(fileName));

        char inBuf[80];
        bsls::Types::Int64 bytesRead = 0;
        rand.reset();
        mileStone = deltaMileStone;

        ASSERT(0 == Util::seek(fd, 0, Util::e_SEEK_FROM_BEGINNING));

        for (;;) {
            int rc = Util::read(fd, inBuf, 80);
            ASSERT(80 == rc);
            ASSERT(0 == bsl::memcmp(record, inBuf, 63));

            rand.munge();
            ASSERT(0 == bsl::memcmp(inBuf + 63, rand.display(), 16));

            ASSERT('\n' == inBuf[79]);

            bytesRead += 80;

            if (bytesRead >= mileStone) {
                cout << bytesRead << " read -- last: " << rand.display() <<
                                                                          endl;
                if (bytesRead >= fiveGig) {
                    break;
                }
                mileStone += deltaMileStone;
            }
        }
        ASSERT(fiveGig == bytesRead);
        ASSERT(bytesWritten == bytesRead);
        ASSERT(Util::seek(fd, 0, Util::e_SEEK_FROM_CURRENT) == bytesRead);
        ASSERT(Util::seek(fd, 0, Util::e_SEEK_FROM_END)     == bytesRead);

        cout << "Reading done\n";

        ASSERT(0 == Util::close(fd));

        ASSERT(Util::getFileSize(fileName) == fiveGig);

        {
            int rc = Util::remove(fileName);
            ASSERT(0 == rc);
        }
      }  break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    ASSERT(0 == Obj::setWorkingDirectory(".."));
    LOOP_ASSERT(mainRoot, Obj::exists(mainRoot));

    // Sometimes this delete won't work because of '.nfs*' gremlin files that
    // mysteriously get created in the directory.  Leave the directory behind
    // and move on.  Also remove twice, because sometimes the first 'remove'
    // 'sorta' fails -- it returns a negative status after successfully killing
    // the gremlin file.  Worst case, leave the file there to be cleaned up
    // in a sweep later.

    Obj::remove(mainRoot, true);
    Obj::remove(mainRoot, true);

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "."
             << endl;
    }
    return testStatus;
}

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2014
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
