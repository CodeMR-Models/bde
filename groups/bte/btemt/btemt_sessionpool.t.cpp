// btemt_sessionpool.t.cpp                                            -*-C++-*-
#include <btemt_sessionpool.h>

#include <btemt_asyncchannel.h>
#include <btemt_channelpool.h>
#include <btemt_message.h>
#include <btemt_session.h>

#include <bcema_blobutil.h>
#include <bcema_testallocator.h>
#include <bcemt_thread.h>
#include <bcemt_barrier.h>
#include <bcemt_semaphore.h>

#include <bdef_bind.h>
#include <bdef_function.h>
#include <bdef_placeholder.h>
#include <bdef_memfn.h>
#include <bslma_defaultallocatorguard.h>
#include <bslma_allocator.h>
#include <bslma_default.h>

#include <bteso_ipv4address.h>
#include <bteso_inetstreamsocketfactory.h>
#include <bteso_socketoptions.h>
#include <bteso_streamsocket.h>

#include <btemt_channelpoolchannel.h>

#include <bsls_atomic.h>

#include <bsl_iostream.h>
#include <bsl_sstream.h>
#include <bsl_cstdlib.h>     // atoi()

using namespace BloombergLP;
using namespace bsl;
using namespace bdef_PlaceHolders;

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// This test driver has grown in an ad-hoc manner and is still incomplete.  A
// thorough review and addition of test cases is still required, but the added
// test cases provide confidence that the core functionality is working.
//
// We begin with a couple of test cases that ensure that a session pool can be
// created, started, and can listen on a specified port, and that data sent to
// that port is received by the session pool.  Next, we test that using blobs
// for data reads works as expected.  After that we test individual
// functionality caused by changes to fix bugs.  The accumulation of all these
// test cases test a significant portion of the component's functionality.
//-----------------------------------------------------------------------------
// CREATORS
// [ 2] btemt_SessionPool(config, poolStateCallback, bslma::Allocator *ta = 0);
// [ 3] btemt_SessionPool(config, poolStateCallback, useBlobs, *ta = 0);
// [ 2] ~btemt_SessionPool();
//
// MANIPULATORS
// [13] int start();
// [13] int stop();
// [ 9] int stopAndRemoveAllSessions();
// [12] int connect(host, ...., *socketOptions);
// [12] int connect(serverAddr, ...., *socketOptions);
// [ 5] int listen(*h, sscb, port, backlog, *factory, *data, *options);
// [ 4] int listen(*h, sscb, port, backlog, reuse, *factory, *data, *options);
// [  ] int listen(*h, sscb, endpoint, backlog, *factory, *data, *options);
// [  ] int listen(*h, sscb, endpoint, backlog, reuse, *factory, *data, *opts);
// [  ] int closeHandle(int handle);
// [12] int connect(*h, sscb, *name, port, numAtts, time, *s, *f, *data, mode);
// [12] int connect(*h, sscb, endpoint, numAtts, time, *s, *f, *userdata);
// [11] int connect(*h, cb, *name, port, atts, time, *f, *data, mode, *o, *la);
// [11] int connect(*h, sscb, endpoint, numAtts, time, *f, *userdata, *o, *la);
// [  ] int import(*h, sscb, *streamSocket, *factory, *sessionFactory, *data);
// [  ] int import(*h, sscb, *streamSocket, *sessionFactory, *userData);
// [10] int setWriteCacheWatermarks(handle, int lowWatermark, int hiWatermark);
//
// ACCESSORS
// [  ] const btemt_ChannelPoolConfiguration& config() const;
// [  ] void getChannelHandleStatistics(*handleInfo) const;
// [ 6] int numSessions() const;
// [ 4] int portNumber(int handle) const;
//
// BUG FIXES
// [ 8] Testing DRQS 28731692
// [ 7] Testing DRQS 29067989
// [ 6] Testing DRQS 24968477
// [ 5] Testing DRQS 20535695
// [ 4] Testing DRQS 22373213
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [14] USAGE EXAMPLE
//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;

static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        bsl::cout << "Error " << __FILE__ << "(" << i << "): " << s
                  << "    (failed)" << bsl::endl;
        if (0 <= testStatus && testStatus <= 100) ++testStatus;
    }
}

#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }

//=============================================================================
//                  STANDARD BDE LOOP-ASSERT TEST MACROS
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\n";aSsErT(1, #X, __LINE__); }}

#define LOOP2_ASSERT(I,J,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " \
              << J << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP3_ASSERT(I,J,K,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " << J << "\t"\
              << #K << ": " << K << "\n"; aSsErT(1, #X, __LINE__); } }

#define LOOP4_ASSERT(I,J,K,L,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " << J << \
       "\t" << #K << ": " << K << "\t" << #L << ": " << L << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP5_ASSERT(I,J,K,L,M,X) { \
   if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " << J << "\t" << \
       #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\n"; \
       aSsErT(1, #X, __LINE__); } }

#define LOOP6_ASSERT(I,J,K,L,M,N,X) { \
   if (!(X)) { bsl::cout << #I << ": " << I << "\t" << #J << ": " << J << "\t"\
       << #K << ": " << K << "\t" << #L << ": " << L << "\t" << \
       #M << ": " << M << "\t" << #N << ": " << N << "\n"; \
       aSsErT(1, #X, __LINE__); } }


//=============================================================================
//                  SEMI-STANDARD TEST OUTPUT MACROS
//-----------------------------------------------------------------------------
#define P(X) bsl::cout << #X " = " << (X) << bsl::endl; // Print identifier
                                                        // and value.
#define Q(X) bsl::cout << "<| " #X " |>" << bsl::endl;  // Quote identifier
                                                        // literally.
#define NL() bsl::cout << bsl::endl;                    // End of line
#define P_(X) bsl::cout << #X " = " << (X) << ", "<< bsl::flush; // P(X)
                                                                 // without
                                                                 // '\n'
#define T_()  bsl::cout << '\t' << bsl::flush;        // Print tab w/o newline
#define L_ __LINE__                           // current Line number

// The following macros facilitate thread-safe streaming to standard output.

#define MTCOUT   coutMutex.lock(); { bsl::cout \
                                           << bcemt_ThreadUtil::selfIdAsInt() \
                                           << ": "
#define MTENDL   bsl::endl;  } coutMutex.unlock()

#define DUMMYBRACE {  //keep context-aware editors happy about the "}" below

#define MTFLUSH  bsl::flush; } coutMutex.unlock()

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

typedef btemt_SessionPool                         Obj;
typedef bteso_SocketOptions                       SocketOptions;
typedef btemt_AsyncChannel::BlobBasedReadCallback BlobReadCallback;
typedef bteso_StreamSocketFactoryDeleter          SocketFactoryDeleter;

static int verbose = 0;
static int veryVerbose = 0;
static int veryVeryVerbose = 0;

static bcemt_Mutex coutMutex;

//=============================================================================
//                  SUPPORT CLASSES AND FUNCTIONS USED FOR TESTING
//-----------------------------------------------------------------------------
static
bteso_IPv4Address getLocalAddress() {
    // On Cygwin, binding to bteso_IPv4Address() doesn't seem to work.
    // Wants to bind to localhost/127.0.0.1.  
#ifdef BSLS_PLATFORM_OS_CYGWIN
    return bteso_IPv4Address("127.0.0.1", 0);
#else
    return bteso_IPv4Address();
#endif
}

namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE {

void poolStateCallback(int reason, int source, void *userData)
{
    if (veryVerbose) {
        MTCOUT << "Pool state changed: (" << reason << ", " << source
               << ") " << MTENDL;
    }
}

void poolStateCallbackWithBarrier(int            state,
                                  int            source,
                                  void          *userData,
                                  int           *poolState,
                                  bcemt_Barrier *barrier)
{
    if (veryVerbose) {
        MTCOUT << "Pool state callback called with"
               << " State: " << state
               << " Source: "  << source << MTENDL;
    }
    *poolState = state;
    barrier->wait();
}

void sessionStateCallback(int            state,
                          int            handle,
                          btemt_Session *session,
                          void          *userData)
{
    switch(state) {
      case btemt_SessionPool::SESSION_DOWN: {
        if (veryVerbose) {
            MTCOUT << "Client from "
                   << session->channel()->peerAddress()
                   << " has disconnected."
                   << MTENDL;
        }
      } break;
      case btemt_SessionPool::SESSION_UP: {
        if (veryVerbose) {
            MTCOUT << "Client connected from "
                   << session->channel()->peerAddress()
                   << MTENDL;
        }
      } break;
    }
}

void sessionStateCallbackWithBarrier(int            state,
                                     int            handle,
                                     btemt_Session *session,
                                     void          *userData,
                                     bcemt_Barrier *barrier)
{
    switch(state) {
      case btemt_SessionPool::SESSION_DOWN: {
        if (veryVerbose) {
            MTCOUT << "Client from "
                   << session->channel()->peerAddress()
                   << " has disconnected."
                   << MTENDL;
        }
      } break;
      case btemt_SessionPool::SESSION_UP: {
        if (veryVerbose) {
            MTCOUT << "Client connected from "
                   << session->channel()->peerAddress()
                   << MTENDL;
        }
        barrier->wait();
      } break;
    }
}

void sessionStateCallbackWithCounter(int              state,
                                     int              handle,
                                     btemt_Session   *session,
                                     void            *userData,
                                     bsls::AtomicInt *numUpConnections)
{
    switch(state) {
      case btemt_SessionPool::SESSION_DOWN: {
        if (veryVerbose) {
            MTCOUT << "Client from "
                   << session->channel()->peerAddress()
                   << " has disconnected."
                   << MTENDL;
        }
      } break;
      case btemt_SessionPool::SESSION_UP: {
        if (veryVerbose) {
            MTCOUT << "Client connected from "
                   << session->channel()->peerAddress()
                   << MTENDL;
        }
        ++*numUpConnections;
      } break;
    }
}

void readCbWithBlob(int         result,
                    int        *numNeeded,
                    bcema_Blob *data,
                    int         channelId,
                    bcema_Blob *blob)
{
    if (result) {
        // Session is going down.

        return;
    }

    ASSERT(numNeeded);
    ASSERT(data);
    ASSERT(0 < data->length());

    blob->moveAndAppendDataBuffers(data);

    *numNeeded = 1;
}

void readCbWithBarrier(int            result,
                       int           *numNeeded,
                       bcema_Blob    *data,
                       int            channelId,
                       bcemt_Barrier *barrier)
{
    if (result) {
        // Session is going down.

        return;
    }

    *numNeeded = 1;
    data->removeAll();
    barrier->wait();
}

void readCbWithCountAndBarrier(int            result,
                               int           *numNeeded,
                               bcema_Blob    *data,
                               int            channelId,
                               int           *cbCount,
                               bcemt_Barrier *barrier)
{
    if (result) {
        // Session is going down.

        return;
    }

    *numNeeded = 1;
    data->removeAll();
    ++*cbCount;
    barrier->wait();
}

                            // =================
                            // class TestSession
                            // =================


class TestSession : public btemt_Session {
    // This class is a concrete implementation of the 'btemt_Session' protocol
    // to use along with 'Tester' objects.

    // DATA
    bool                d_useBlobReadCb;
    btemt_AsyncChannel *d_channel_p;
    BlobReadCallback   *d_callback_p;

  private:
    // NOT IMPLEMENTED
    TestSession(const TestSession&);
    TestSession& operator=(const TestSession&);

    void blobReadCb(int         result,
                    int        *numNeeded,
                    bcema_Blob *blob,
                    int         channelId);
        // Read callback for session pool.

    void dataMsgReadCb(int                   state,
                       int                  *numConsumed,
                       int                  *numNeeded,
                       const btemt_DataMsg&  msg);

  public:
    // CREATORS
    TestSession(bool                useBlobReadCb,
                btemt_AsyncChannel *channel,
                BlobReadCallback   *callback);
        // Create a new 'TestSession' object for the specified 'channel'.

    ~TestSession();
        // Destroy this object.

    // MANIPULATORS
    virtual int start();
        // Begin the asynchronous operation of this session.

    virtual int stop();
        // Stop the operation of this session.

    // ACCESSORS
    virtual btemt_AsyncChannel *channel() const;
        // Return the channel associate with this session.
};

                            // -----------------
                            // class TestSession
                            // -----------------

// PRIVATE MANIPULATORS
void TestSession::blobReadCb(int         result,
                             int        *numNeeded,
                             bcema_Blob *blob,
                             int         channelId)
{
    if (result) {
        // Session is going down.

        d_channel_p->close();
        return;
    }

    if (0 != blob->length()) {
        ASSERT(0 == d_channel_p->write(*blob));
    }

    blob->removeAll();

    *numNeeded = 1;
}

void TestSession::dataMsgReadCb(int                   state,
                                int                  *numConsumed,
                                int                  *numNeeded,
                                const btemt_DataMsg&  msg)
{
    if (state) {
        // Session is going down.

        d_channel_p->close();
        return;
    }

    ASSERT(numConsumed);
    ASSERT(msg.data());
    ASSERT(0 < msg.data()->length());

    ASSERT(0 == d_channel_p->write(msg));

    *numConsumed = msg.data()->length();
    *numNeeded   = 1;
}

// CREATORS
TestSession::TestSession(bool                useBlobReadCb,
                         btemt_AsyncChannel *channel,
                         BlobReadCallback   *callback)
: d_useBlobReadCb(useBlobReadCb)
, d_channel_p(channel)
, d_callback_p(callback)
{
}

TestSession::~TestSession()
{
}

// MANIPULATORS
int TestSession::start()
{
    if (veryVerbose) {
        MTCOUT << "Session started" << MTENDL;
    }

    if (d_useBlobReadCb) {
        btemt_AsyncChannel::BlobBasedReadCallback callback =
                            d_callback_p
                            ? *d_callback_p
                            : bdef_MemFnUtil::memFn(&TestSession::blobReadCb,
                                                    this);

        d_channel_p->read(1, callback);
    }
    else {
        btemt_AsyncChannel::ReadCallback callback =
                      bdef_MemFnUtil::memFn(&TestSession::dataMsgReadCb, this);

        d_channel_p->read(1, callback);
    }

    return 0;
}

int TestSession::stop()
{
    if (veryVerbose) {
        MTCOUT << "Session stopped" << MTENDL;
    }

    d_channel_p->close();
    return 0;
}

// ACCESSORS
btemt_AsyncChannel *TestSession::channel() const
{
    return d_channel_p;
}

                    // =================
                    // class TestFactory
                    // =================

class TestFactory : public btemt_SessionFactory {
    // This class is a concrete implementation of the 'btemt_SessionFactory'
    // that simply allocates 'TestSession' objects.  No specific allocation
    // strategy (such as pooling) is implemented.

    // DATA
    bool              d_useBlobReadCb; // use blobs
    BlobReadCallback *d_callback_p;    // read callback (held, not owned)
    bslma::Allocator *d_allocator_p;   // memory allocator (held, not owned)

  public:
    // TRAITS
    BSLALG_DECLARE_NESTED_TRAITS(TestFactory,
                                 bslalg::TypeTraitUsesBslmaAllocator);

    // CREATORS
    TestFactory(bool              useBlobReadCb = true,
                BlobReadCallback *callback = 0,
                bslma::Allocator *basicAllocator = 0);
        // Create a new 'TestFactory' object.  Optionally specify a
        // 'basicAllocator' used to supply memory.  If 'basicAllocator' is 0,
        // the currently installed default allocator is used.

    virtual ~TestFactory();
        // Destroy this factory.

    // MANIPULATORS
    virtual void allocate(btemt_AsyncChannel                    *channel,
                          const btemt_SessionFactory::Callback&  callback);
        // Asynchronously allocate a 'btemt_Session' object for the
        // specified 'channel', and invoke the specified 'callback' with
        // this session.

    virtual void deallocate(btemt_Session *session);
        // Deallocate the specified 'session'.
};

                        // -----------------
                        // class TestFactory
                        // -----------------

// CREATORS
TestFactory::TestFactory(bool              useBlobReadCb,
                         BlobReadCallback *callback,
                         bslma::Allocator *basicAllocator)
: d_useBlobReadCb(useBlobReadCb)
, d_callback_p(callback)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

TestFactory::~TestFactory()
{
}

// MANIPULATORS
void TestFactory::allocate(btemt_AsyncChannel                    *channel,
                           const btemt_SessionFactory::Callback&  callback)
{
    if (veryVerbose) {
        MTCOUT << "Allocate factory called: " << MTENDL;
    }

    TestSession *session = new (*d_allocator_p) TestSession(d_useBlobReadCb,
                                                            channel,
                                                            d_callback_p);

    callback(0, session);
}

void TestFactory::deallocate(btemt_Session *session)
{
    if (veryVerbose) {
        MTCOUT << "Deallocate factory called: " << MTENDL;
    }

    d_allocator_p->deleteObjectRaw(session);
}

}  // close namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE

namespace BTEMT_SESSION_POOL_SETTING_SOCKETOPTIONS {

using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

int createConnection(
                Obj                                          *sessionPool,
                btemt_SessionPool::SessionStateCallback      *sessionStateCb, 
                btemt_SessionFactory                         *sessionFactory,
                bteso_StreamSocket<bteso_IPv4Address>        *serverSocket,
                SocketOptions                                *socketOptions,
                bteso_StreamSocketFactory<bteso_IPv4Address> *socketFactory,
                const bteso_IPv4Address                      *ipAddress)
{
    ASSERT(0 == serverSocket->bind(getLocalAddress()));
    ASSERT(0 == serverSocket->listen(1));

    bteso_IPv4Address serverAddr;
    ASSERT(0 == serverSocket->localAddress(&serverAddr));

    int handleBuffer;
    if (socketOptions) {
        return sessionPool->connect(&handleBuffer,
                                    *sessionStateCb,
                                    serverAddr,
                                    1,
                                    bdet_TimeInterval(1),
                                    sessionFactory,
                                    0,
                                    socketOptions,
                                    ipAddress);
    } else {
        BSLS_ASSERT_OPT(socketFactory); // test invariant

        typedef bteso_StreamSocketFactoryDeleter Deleter;

        bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
            clientSocket(socketFactory->allocate(),
                         socketFactory,
                         &Deleter::deleteObject<bteso_IPv4Address>);

        const int rc = clientSocket->bind(*ipAddress);
        if (rc) {
            return rc;                                                // RETURN
        }

        return sessionPool->connect(&handleBuffer,
                                    *sessionStateCb,
                                    serverAddr,
                                    1,
                                    bdet_TimeInterval(1),
                                    &clientSocket,
                                    sessionFactory,
                                    0);
    }
}

}  // close namespace BTEMT_SESSION_POOL_SETTING_SOCKETOPTIONS


namespace BTEMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS {

using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

bcemt_Mutex                                           mapMutex;
bsl::map<int, btemt_AsyncChannel *>                   sourceIdToChannelMap;
typedef bsl::map<int, btemt_AsyncChannel *>::iterator MapIter;

void sessionStateCallbackUsingChannelMapAndCounter(
                                             int              state,
                                             int              handle,
                                             btemt_Session   *session,
                                             void            *userData,
                                             bsls::AtomicInt *numUpConnections)
{
    switch(state) {
      case btemt_SessionPool::SESSION_DOWN: {
          if (veryVerbose) {
              MTCOUT << "Client from "
                     << session->channel()->peerAddress()
                     << " has disconnected."
                     << MTENDL;
          }
      } break;
      case btemt_SessionPool::SESSION_UP: {
          if (veryVerbose) {
              MTCOUT << "Client connected from "
                     << session->channel()->peerAddress()
                     << MTENDL;
          }
          {
              bcemt_LockGuard<bcemt_Mutex> guard(&mapMutex);
              sourceIdToChannelMap[handle] = session->channel();
          }
          ++*numUpConnections;
      } break;
    }
}

const int NUM_BYTES = 1024 * 1024;
const int NUM_THREADS = 5;

bsl::vector<bteso_StreamSocket<bteso_IPv4Address> *>
                                                    clientSockets(NUM_THREADS);
bteso_InetStreamSocketFactory<bteso_IPv4Address>    factory;

struct ConnectData {
    int                d_index;
    int                d_numBytes;
    bteso_IPv4Address  d_serverAddress;
};

void *connectFunction(void *args)
{
    ConnectData              data      = *(const ConnectData *) args;
    const int                INDEX     = data.d_index;
    const int                NUM_BYTES = data.d_numBytes;
    const bteso_IPv4Address  ADDRESS   = data.d_serverAddress;

    bteso_StreamSocket<bteso_IPv4Address> *socket = factory.allocate();
    clientSockets[INDEX] = socket;

    ASSERT(0 == socket->connect(ADDRESS));

    bsl::vector<char> buffer(NUM_BYTES);

    int numRemaining = NUM_BYTES;
    do {
        int rc = socket->read(buffer.data(), numRemaining);
        if (rc < 0) {
            if (rc == bteso_SocketHandle::BTESO_ERROR_WOULDBLOCK) {
                continue;
            }
            socket->shutdown(bteso_Flag::BTESO_SHUTDOWN_BOTH);
            return 0;                                                 // RETURN
        }
        
        numRemaining -= rc;

        if (0 == socket->connectionStatus()) {
            rc = socket->write(buffer.data(), numRemaining);

            if (rc < 0) {
                socket->shutdown(bteso_Flag::BTESO_SHUTDOWN_BOTH);
                return 0;                                             // RETURN
            }

            bcemt_ThreadUtil::microSleep(1000 , 0);
        }
        else {
            return 0;                                                 // RETURN
        }
    } while (numRemaining > 0);
    return 0;
}

bsl::vector<bteso_StreamSocket<bteso_IPv4Address> *>
                                                    serverSockets(NUM_THREADS);

struct ListenData {
    int              d_index;
    int              d_numBytes;
    bsls::AtomicInt *d_numUpConnections_p;
};

void *listenFunction(void *args)
{
    ListenData       data             = *(const ListenData *) args;
    const int        INDEX            = data.d_index;
    const int        NUM_BYTES        = data.d_numBytes;
    bsls::AtomicInt *numUpConnections = data.d_numUpConnections_p;  

    bteso_StreamSocket<bteso_IPv4Address> *serverSocket = factory.allocate();
    serverSockets[INDEX] = serverSocket;

    ASSERT(0 == serverSocket->bind(getLocalAddress()));
    ASSERT(0 == serverSocket->listen(1));

    ++*numUpConnections;

    bteso_StreamSocket<bteso_IPv4Address> *acceptSocket;
    ASSERT(!serverSocket->accept(&acceptSocket));
    ASSERT(0 ==
            acceptSocket->setBlockingMode(bteso_Flag::BTESO_BLOCKING_MODE));

    bsl::vector<char> buffer(NUM_BYTES);

    int numRemaining = NUM_BYTES;
    do {
        int rc = acceptSocket->read(buffer.data(), numRemaining);
        if (rc < 0) {
            if (rc == bteso_SocketHandle::BTESO_ERROR_WOULDBLOCK) {
                continue;
            }
            acceptSocket->shutdown(bteso_Flag::BTESO_SHUTDOWN_BOTH);
            return 0;                                                 // RETURN
        }

        numRemaining -= rc;

        if (0 == acceptSocket->connectionStatus()) {
            rc = acceptSocket->write(buffer.data(), numRemaining);

            if (rc < 0) {
                acceptSocket->shutdown(bteso_Flag::BTESO_SHUTDOWN_BOTH);
                return 0;                                             // RETURN
            }

            bcemt_ThreadUtil::microSleep(1000 , 0);
        }
        else {
            return 0;                                                 // RETURN
        }
    } while (numRemaining > 0);
    return 0;
}

bsls::AtomicInt numUpConnections(0);

void runTestFunction(bcemt_ThreadUtil::Handle                *connectThreads,
                     bcemt_ThreadUtil::Handle                *listenThreads,
                     btemt_SessionPool                       *pool,
                     btemt_SessionPool::SessionStateCallback *sessionStateCb,
                     TestFactory                             *sessionFactory,
                     const bcema_Blob&                        dataBlob)
{
    bsl::vector<int> serverHandles(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; ++i) {
        ASSERT(0 == pool->listen(&serverHandles[i],
                                 *sessionStateCb,
                                 0,
                                 5,
                                 sessionFactory));
    }

    ConnectData connectData[NUM_THREADS];
    const int   SIZE = 1024 * 1024; // 1 MB

    for (int i = 0; i < NUM_THREADS; ++i) {
        connectData[i].d_index     = i;
        connectData[i].d_numBytes  = SIZE;

        const int PORTNUM = pool->portNumber(serverHandles[i]);
        connectData[i].d_serverAddress = bteso_IPv4Address("127.0.0.1",
                                                           PORTNUM);

        ASSERT(0 == bcemt_ThreadUtil::create(&connectThreads[i],
                                             &connectFunction,
                                             (void *) &connectData[i]));
    }

    while (numUpConnections < NUM_THREADS) {
        bcemt_ThreadUtil::microSleep(50, 0);
    }

    numUpConnections = 0;

    ListenData listenData[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        listenData[i].d_index              = i;
        listenData[i].d_numBytes           = SIZE;
        listenData[i].d_numUpConnections_p = &numUpConnections;

        ASSERT(0 == bcemt_ThreadUtil::create(&listenThreads[i],
                                             &listenFunction,
                                             (void *) &listenData[i]));
    }

    while (numUpConnections < NUM_THREADS) {
        bcemt_ThreadUtil::microSleep(50, 0);
    }

    numUpConnections = 0;

    bsl::vector<int> clientHandles(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; ++i) {
        bteso_IPv4Address serverAddr;
        ASSERT(0 == serverSockets[i]->localAddress(&serverAddr));

        ASSERT(0 == pool->connect(&clientHandles[i],
                                  *sessionStateCb,
                                  serverAddr,
                                  10,
                                  bdet_TimeInterval(1),
                                  sessionFactory));
    }

    while (numUpConnections < NUM_THREADS) {
        bcemt_ThreadUtil::microSleep(50, 0);
    }

    mapMutex.lock();
    for (int i = 0; i < NUM_THREADS; ++i) {
        MapIter iter = sourceIdToChannelMap.find(serverHandles[i]);
        if (iter != sourceIdToChannelMap.end()) {
            ASSERT(0 == iter->second->write(dataBlob));
        }

        iter = sourceIdToChannelMap.find(clientHandles[i]);
        if (iter != sourceIdToChannelMap.end()) {
            ASSERT(0 == iter->second->write(dataBlob));
        }
    }
    mapMutex.unlock();
}

}  // close namespace BTEMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS


namespace BTEMT_SESSION_POOL_DRQS_29067989 {

using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

static bslma::TestAllocator testAllocator;
static int callbackCount = 0;

static int maxLength = 0;
static int maxSize = 0;
static int maxExtra = 0;
static int maxNumBuffers = 0;
static int maxNumDataBuffers = 0;

enum {
    PAYLOAD_SIZE = 320,
    HALF_PAYLOAD_SIZE = 160
};

void readCbWithMetrics(int              result,
                       int             *numNeeded,
                       bcema_Blob      *blob,
                       int              channelId,
                       int              numBytesToRead,
                       bcemt_Semaphore *semaphore)
{
    static int numBytesRead = 0;

    if (result) {
        // Session is going down.

        return;
    }

    ++callbackCount;

    ASSERT(numNeeded);
    ASSERT(blob);
    ASSERT(0 < blob->length());

    numBytesRead += blob->length();

    int consume = blob->length();
    if (0 == callbackCount % 2) {
        // Every second time leave a bit of data in the input buffer.
        consume -= sizeof(int);
    }

    if (maxLength < blob->length()) {
        maxLength = blob->length();
    }

    if (maxSize < blob->totalSize()) {
        maxSize = blob->totalSize();
    }

    if (maxExtra < (blob->totalSize() - blob->length())) {
        maxExtra = (blob->totalSize() - blob->length());
    }

    if (maxNumDataBuffers < blob->numDataBuffers()) {
        maxNumDataBuffers = blob->numDataBuffers();
    }

    if (maxNumBuffers < blob->numBuffers()) {
        maxNumBuffers = blob->numBuffers();
    }

    bcema_BlobUtil::erase(blob, 0, consume);

    if (numBytesRead < numBytesToRead) {
        *numNeeded = PAYLOAD_SIZE;
    }
    else {
        *numNeeded = 0;
        semaphore->post();
    }
}

}  // close namespace BTEMT_SESSION_POOL_DRQS_29067989

namespace BTEMT_SESSION_POOL_DRQS_20535695 {

using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

                    // ========================
                    // class TestSessionFactory
                    // ========================

class TestSessionFactory : public btemt_SessionFactory {
    // This class is a concrete implementation of the 'btemt_SessionFactory'
    // that simply allocates 'TestSession' objects.  No specific allocation
    // strategy (such as pooling) is implemented.

    // DATA
    bcemt_Barrier *d_barrier_p;

  public:
    // CREATORS
    TestSessionFactory(bcemt_Barrier *barrier);
        // Create a new 'TestSessionFactory' object using the specified
        // 'barrier'.

    virtual ~TestSessionFactory();
        // Destroy this factory.

    // MANIPULATORS
    virtual void allocate(btemt_AsyncChannel                   *channel,
                          const btemt_SessionFactory::Callback& callback);
        // Asynchronously allocate a 'btemt_Session' object for the
        // specified 'channel', and invoke the specified 'callback' with
        // this session.

    virtual void deallocate(btemt_Session *session);
        // Deallocate the specified 'session'.

    btemt_AsyncChannel *channel() const;
        // Return the channel managed by this factory.
};

                        // ------------------------
                        // class TestSessionFactory
                        // ------------------------

// CREATORS
TestSessionFactory::TestSessionFactory(bcemt_Barrier *barrier)
: d_barrier_p(barrier)
{
}

TestSessionFactory::~TestSessionFactory()
{
}

// MANIPULATORS
void TestSessionFactory::allocate(
                               btemt_AsyncChannel                    *channel,
                               const btemt_SessionFactory::Callback&  callback)
{
    if (veryVerbose) {
        MTCOUT << "Allocate factory called: " << MTENDL;
    }

    d_barrier_p->wait();
}

void TestSessionFactory::deallocate(btemt_Session *session)
{
    if (veryVerbose) {
        MTCOUT << "Deallocate factory called: " << MTENDL;
    }
}

btemt_AsyncChannel *TestSessionFactory::channel() const
{
    return 0;
}

}  // close namespace BTEMT_SESSION_POOL_DRQS_20535695

namespace BTEMT_SESSION_POOL_DRQS_44879376 {

                            // ===================
                            // class TesterSession
                            // ===================

class TesterSession : public btemt_Session {
    // This class is a concrete implementation of the 'btemt_Session' protocol
    // to use along with 'Tester' objects.

    // DATA
    bcema_SharedPtr<btemt_AsyncChannel>  d_channel_sp;
    bcemt_Barrier                       *d_barrier_p;

    // PRIVATE MANIPULATORS
    void delayedChannelAccessor(
                          bcema_SharedPtr<btemt_AsyncChannel> spChannel,
                          bteso_IPv4Address                   referenceAddress)
    {
        d_barrier_p->wait();

        ASSERT(spChannel);

        if (veryVerbose) {
            MTCOUT << "Late access of Async Channel, peerAddress = " <<
                    spChannel->peerAddress() << MTENDL;
        }

        ASSERT(spChannel->peerAddress() == referenceAddress);
    }

    void readCb(int                   state,
                int                  *numConsumed,
                int                  *numNeeded,
                const btemt_DataMsg&  msg);
        // Read callback for session pool.

  private:
    // NOT IMPLEMENTED
    TesterSession(const TesterSession&);
    TesterSession& operator=(const TesterSession&);

  public:
    // CREATORS
    TesterSession(const bcema_SharedPtr<btemt_AsyncChannel>&  channel,
                  bcemt_Barrier                              *barrier);
        // Create a new 'TesterSession' object for the specified 'channel'.

    ~TesterSession();
        // Destroy this object.

    // MANIPULATORS
    virtual int start();
        // Begin the asynchronous operation of this session.

    virtual int stop();
        // Stop the operation of this session.

    // ACCESSORS
    virtual btemt_AsyncChannel *channel() const;
        // Return the channel associate with this session.
};

                    // ===================
                    // class TesterFactory
                    // ===================

class TesterFactory : public btemt_SessionFactory {
    // This class is a concrete implementation of the 'btemt_SessionFactory'
    // that simply allocates 'TesterSession' objects.  No specific allocation
    // strategy (such as pooling) is implemented.

    // DATA
    int                              d_mode;

    bcemt_Barrier                   *d_barrier_p;   // held not owned

    TesterSession                   *d_session_p;   // held not owned

    bslma::Allocator                *d_allocator_p; // memory allocator (held,
                                                    // not owned)

  public:

    enum { LISTENER = 0, CONNECTOR };

    // TRAITS
    BSLALG_DECLARE_NESTED_TRAITS(TesterFactory,
                                 bslalg::TypeTraitUsesBslmaAllocator);

    // CREATORS
    TesterFactory(int               mode,
                  bcemt_Barrier    *barrier,
                  bslma::Allocator *basicAllocator = 0);
        // Create a new 'TesterFactory' object of the specified 'mode'.
        // Optionally specify 'basicAllocator' used to supply memory.  If
        // 'basicAllocator' is 0, the currently installed default allocator is
        // used.

    virtual ~TesterFactory();
        // Destroy this factory.

    // MANIPULATORS
    virtual void allocate(const bcema_SharedPtr<btemt_AsyncChannel>& channel,
                          const btemt_SessionFactory::Callback&      callback);
        // Asynchronously allocate a 'btemt_Session' object for the
        // specified 'channel', and invoke the specified 'callback' with
        // this session.

    virtual void allocate(btemt_AsyncChannel                    *channel,
                          const btemt_SessionFactory::Callback&  callback);
        // Asynchronously allocate a 'btemt_Session' object for the
        // specified 'channel', and invoke the specified 'callback' with
        // this session.

    virtual void deallocate(btemt_Session *session);
        // Deallocate the specified 'session'.

    TesterSession *session() const;
        // Return the session managed by this factory.
};

                            // -------------------
                            // class TesterSession
                            // -------------------

// CREATORS
TesterSession::TesterSession(
                           const bcema_SharedPtr<btemt_AsyncChannel>&  channel,
                           bcemt_Barrier                              *barrier)
: d_channel_sp(channel)
, d_barrier_p(barrier)
{
}

TesterSession::~TesterSession()
{
}

// MANIPULATORS
void TesterSession::readCb(int                  state,
                           int                 *numConsumed,
                           int                 *numNeeded,
                           const btemt_DataMsg& msg)
{
    if (veryVerbose) {
        MTCOUT << "Read callback called with: " << state << MTENDL;
    }

    if (state) {
        d_channel_sp->close();
        return;
    }

    ASSERT(0 == d_channel_sp->write(msg));

    *numConsumed = msg.data()->length();
    *numNeeded   = 1;

    bcemt_ThreadUtil::Handle handle(bcemt_ThreadUtil::invalidHandle());
    ASSERT(0 == bcemt_ThreadUtil::create(
                &handle,
                bdef_BindUtil::bind(&TesterSession::delayedChannelAccessor,
                                    this,
                                    d_channel_sp,
                                    d_channel_sp->peerAddress())));
    bcemt_ThreadUtil::detach(handle);
}

int TesterSession::start()
{
    if (veryVerbose) {
        MTCOUT << "Session started" << MTENDL;
    }

    bdef_Function<void (*)(int,
                           int*,
                           int*,
                           const BloombergLP::btemt_DataMsg&)> f =
                              bdef_MemFnUtil::memFn(&TesterSession::readCb,
                                                    this);

    return d_channel_sp->read(1, f);
}

int TesterSession::stop()
{
    if (veryVerbose) {
        MTCOUT << "Session stopped" << MTENDL;
    }

    d_channel_sp->close();
    d_channel_sp.reset();

    d_barrier_p->wait();

    return 0;
}

// ACCESSORS
btemt_AsyncChannel *TesterSession::channel() const
{
    return d_channel_sp.ptr();
}

                        // -------------------
                        // class TesterFactory
                        // -------------------

// CREATORS
TesterFactory::TesterFactory(int               mode,
                             bcemt_Barrier    *barrier,
                             bslma::Allocator *basicAllocator)
: d_mode(mode)
, d_session_p(0)
, d_barrier_p(barrier)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
}

TesterFactory::~TesterFactory()
{
}

// MANIPULATORS
void TesterFactory::allocate(
                           const bcema_SharedPtr<btemt_AsyncChannel>& channel,
                           const btemt_SessionFactory::Callback&      callback)
{
    if (veryVerbose) {
        MTCOUT << "TesterFactory::allocate called: " << MTENDL;
        if (LISTENER == d_mode) {
            MTCOUT << "LISTENER" << MTENDL;
        }
        else {
            MTCOUT << "CONNECTOR" << MTENDL;
        }
    }

    d_session_p = new (*d_allocator_p) TesterSession(channel, d_barrier_p);

    callback(0, d_session_p);
}

void TesterFactory::allocate(btemt_AsyncChannel                    *channel,
                             const btemt_SessionFactory::Callback&  callback)
{
    ASSERT(false); // shouldn't get called
}

void TesterFactory::deallocate(btemt_Session *session)
{
    d_allocator_p->deleteObject(session);
}

TesterSession *TesterFactory::session() const
{
    return d_session_p;
}

                        // ------------
                        // class Tester
                        // ------------

class Tester {
    btemt_ChannelPoolConfiguration  d_config;          // pool configuration

    btemt_SessionPool              *d_sessionPool_p;   // managed pool (owned)

    TesterFactory                   d_sessionFactory;  // factory

    int                             d_portNumber;      // port on which this
                                                       // echo server is
                                                       // listening

    bslma::Allocator               *d_allocator_p;     // memory allocator
                                                       // (held)

  public:

    enum { LISTENER = 0, CONNECTOR };

    // CREATORS
    Tester(int                       mode,
           bcemt_Barrier            *barrier,
           const bteso_IPv4Address&  endPointAddr,
           bool                      useBlobBasedReads = false,
           bslma::Allocator         *basicAllocator = 0);
    ~Tester();

    void poolStateCb(int reason, int source, void *userData);
        // Indicates the status of the whole pool.

    void sessionStateCb(int            state,
                        int            handle,
                        btemt_Session *session,
                        void          *userData);
        // Per-session state.

    int portNumber() const;

    TesterSession *session() const { return d_sessionFactory.session(); }
};

// CREATORS
Tester::Tester(int                       mode,
               bcemt_Barrier            *barrier,
               const bteso_IPv4Address&  endPointAddr,
               bool                      useBlobBasedReads,
               bslma::Allocator         *basicAllocator)
: d_config()
, d_sessionPool_p()
, d_sessionFactory(mode, barrier, basicAllocator)
, d_portNumber(bteso_IPv4Address::BTESO_ANY_PORT)
, d_allocator_p(bslma::Default::allocator(basicAllocator))
{
    d_config.setMaxThreads(4);                  // 4 I/O threads
    d_config.setMaxConnections(5);
    d_config.setReadTimeout(5.0);               // in seconds
    d_config.setMetricsInterval(10.0);          // seconds
    d_config.setMaxWriteCache(1<<10);           // 1Mb
    d_config.setIncomingMessageSizes(1, 100, 1024);

    typedef btemt_SessionPool::SessionPoolStateCallback SessionPoolStateCb;

    SessionPoolStateCb poolStateCb = bdef_MemFnUtil::memFn(
                                          &Tester::poolStateCb,
                                          this);

    d_sessionPool_p = new (*d_allocator_p)
                             btemt_SessionPool(d_config,
                                               poolStateCb,
                                               useBlobBasedReads,
                                               basicAllocator);

    btemt_SessionPool::SessionStateCallback sessionStateCb =
                 bdef_MemFnUtil::memFn(&Tester::sessionStateCb,
                                       this);

    ASSERT(0 == d_sessionPool_p->start());
    int handle;
    if (LISTENER == mode) {
        ASSERT(0 == d_sessionPool_p->listen(&handle,
                                            sessionStateCb,
                                            d_portNumber,
                                            5,
                                            &d_sessionFactory));

        d_portNumber = d_sessionPool_p->portNumber(handle);
    }
    else {
        ASSERT(0 == d_sessionPool_p->connect(&handle,
                                             sessionStateCb,
                                             endPointAddr,
                                             5,
                                             bdet_TimeInterval(1),
                                             &d_sessionFactory));
        d_portNumber = endPointAddr.portNumber();
    }
}

Tester::~Tester()
{
    d_sessionPool_p->stop();
    d_allocator_p->deleteObject(d_sessionPool_p);
}

void Tester::poolStateCb(int reason, int source, void *userData)
{
    if (veryVerbose) {
        MTCOUT << "Pool state changed: (" << reason << ", " << source << ") "
               << MTENDL;
    }
}

void Tester::sessionStateCb(int            state,
                            int            handle,
                            btemt_Session *session,
                            void          *userData)
{
    switch (state) {
      case btemt_SessionPool::SESSION_DOWN: {
          if (veryVerbose) {
              MTCOUT << "Client from "
                     << session->channel()->peerAddress()
                     << " has disconnected."
                     << MTENDL;
          }
      } break;
      case btemt_SessionPool::SESSION_UP: {
          if (veryVerbose) {
              MTCOUT << "Client connected from "
                     << session->channel()->peerAddress()
                     << MTENDL;
          }
      } break;
    }
}

int Tester::portNumber() const
{
    return d_portNumber;
}

}  // end namespace BTEMT_SESSION_POOL_DRQS_44879376

//=============================================================================
//                                USAGE EXAMPLE
//-----------------------------------------------------------------------------

namespace BTEMT_SESSION_POOL_USAGE_EXAMPLE {

///Usage example
///-------------
// The following example implements a simple echo server.  This server accepts
// connections, reads what it receives right away from the network stream,
// sends it back and closes the connection.
//
// To set up this server, users must create a concrete session class derived
// from 'btemt_Session' protocol, and a factory for creating instances of this
// concrete session type.  'my_EchoSession' objects are created by a factory
// that must be derived from 'btemt_SessionFactory'.  A 'my_EchoSessionFactory'
// just allocates and deallocates sessions (with no pooling or allocation
// strategy).  This is the simplest form of factory.
//..
    // my_echoserver.h

                            // ====================
                            // class my_EchoSession
                            // ====================

   class my_EchoSession : public btemt_Session {
       // This class is a concrete implementation of the 'btemt_Session'
       // protocol to use along with 'my_EchoServer' objects.

       // DATA
       btemt_AsyncChannel *d_channel_p; // underlying channel (held, not owned)

       // PRIVATE MANIPULATORS
       void readCb(int                   state,
                   int                  *numConsumed,
                   int                  *numNeeded,
                   const btemt_DataMsg&  msg);
           // Read callback for session pool.

     private:
       // NOT IMPLEMENTED
       my_EchoSession(const my_EchoSession&);
       my_EchoSession& operator=(const my_EchoSession&);

     public:
       // CREATORS
       my_EchoSession(btemt_AsyncChannel *channel);
           // Create a new 'my_EchoSession' object for the specified 'channel'.

       ~my_EchoSession();
           // Destroy this object.

       // MANIPULATORS
       virtual int start();
           // Begin the asynchronous operation of this session.

       virtual int stop();
           // Stop the operation of this session.

       // ACCESSORS
       virtual btemt_AsyncChannel *channel() const;
           // Return the channel associate with this session.
    };

                        // ===========================
                        // class my_EchoSessionFactory
                        // ===========================

    class my_EchoSessionFactory : public btemt_SessionFactory {
        // This class is a concrete implementation of the
        // 'btemt_SessionFactory' that simply allocates 'my_EchoSession'
        // objects.  No specific allocation strategy (such as pooling) is
        // implemented.

        bslma::Allocator *d_allocator_p; // memory allocator (held, not owned)

      public:
        // TRAITS
        BSLALG_DECLARE_NESTED_TRAITS(my_EchoSessionFactory,
                                     bslalg::TypeTraitUsesBslmaAllocator);

        // CREATORS
        my_EchoSessionFactory(bslma::Allocator *basicAllocator = 0);
            // Create a new 'my_EchoSessionFactory' object.  Optionally specify
            // 'basicAllocator' used to supply memory.  If 'basicAllocator' is
            // 0, the currently installed default allocator is used.

        virtual ~my_EchoSessionFactory();
           // Destroy this factory.

        // MANIPULATORS
        virtual void allocate(btemt_AsyncChannel                   *channel,
                              const btemt_SessionFactory::Callback& callback);
           // Asynchronously allocate a 'btemt_Session' object for the
           // specified 'channel', and invoke the specified 'callback' with
           // this session.

        virtual void deallocate(btemt_Session *session);
           // Deallocate the specified 'session'.
    };
//..
// The implementations of those session and factory types are rather
// straightforward.  'readCb' will be called when the first byte is received.
// It is in this method that the echo logic is implemented.
//..
    // my_echoserver.cpp

                            // --------------------
                            // class my_EchoSession
                            // --------------------

    // PRIVATE MANIPULATORS
    void my_EchoSession::readCb(int                   state,
                                int                  *numConsumed,
                                int                  *numNeeded,
                                const btemt_DataMsg&  msg)
    {
        if (state) {
            // Session is going down.

            d_channel_p->close();
            return;
        }

        ASSERT(numConsumed);
        ASSERT(msg.data());
        ASSERT(0 < msg.data()->length());

        ASSERT(0 == d_channel_p->write(msg));

        *numConsumed = msg.data()->length();
        *numNeeded   = 1;

        d_channel_p->close(); // close connection.
    }

    // CREATORS
    my_EchoSession::my_EchoSession(btemt_AsyncChannel *channel)
    : d_channel_p(channel)
    {
    }

    my_EchoSession::~my_EchoSession()
    {
    }

    // MANIPULATORS
    int my_EchoSession::start()
    {
        btemt_AsyncChannel::ReadCallback
                        callback(bdef_MemFnUtil::memFn(&my_EchoSession::readCb,
                                                       this));
        return d_channel_p->read(1, callback);
    }

    int my_EchoSession::stop()
    {
        d_channel_p->close();
        return 0;
    }

    // ACCESSORS
    btemt_AsyncChannel *my_EchoSession::channel() const
    {
        return d_channel_p;
    }

                        // ---------------------------
                        // class my_EchoSessionFactory
                        // ---------------------------

    // CREATORS
    my_EchoSessionFactory::my_EchoSessionFactory(
                                              bslma::Allocator *basicAllocator)
    : d_allocator_p(bslma::Default::allocator(basicAllocator))
    {
    }

    my_EchoSessionFactory::~my_EchoSessionFactory()
    {
    }

    // MANIPULATORS
    void
    my_EchoSessionFactory::allocate(
                                btemt_AsyncChannel                   *channel,
                                const btemt_SessionFactory::Callback& callback)
    {
        my_EchoSession *session = new (*d_allocator_p) my_EchoSession(channel);
        callback(0, session);
    }

    void
    my_EchoSessionFactory::deallocate(btemt_Session *session)
    {
        d_allocator_p->deleteObjectRaw(session);
    }
//..
//  We now have all the pieces needed to design and implement our echo server.
//  The server itself owns an instance of the above-defined factory.
//..
    // my_echoserver.h (continued)

                        // ===================
                        // class my_EchoServer
                        // ===================

    class my_EchoServer {
        // This class implements a multi-user multi-threaded echo server.

        // DATA
        btemt_ChannelPoolConfiguration d_config;          // pool
                                                          // configuration

        btemt_SessionPool             *d_sessionPool_p;   // managed pool
                                                          // (owned)

        my_EchoSessionFactory          d_sessionFactory;  // my_EchoSession
                                                          // factory

        int                            d_portNumber;      // port on which this
                                                          // echo server is
                                                          // listening

        bcemt_Mutex                   *d_coutLock_p;      // mutex protecting
                                                          // bsl::cout

        bslma::Allocator              *d_allocator_p;     // memory allocator
                                                          // (held)

        // PRIVATE MANIPULATORS
        void poolStateCb(int reason, int source, void *userData);
            // Indicates the status of the whole pool.

        void sessionStateCb(int            state,
                            int            handle,
                            btemt_Session *session,
                            void          *userData);
            // Per-session state.

      private:
        // NOT IMPLEMENTED
        my_EchoServer(const my_EchoServer& original);
        my_EchoServer& operator=(const my_EchoServer& rhs);

      public:
        // TRAITS
        BSLALG_DECLARE_NESTED_TRAITS(my_EchoServer,
                                     bslalg::TypeTraitUsesBslmaAllocator);

        // CREATORS
        my_EchoServer(bcemt_Mutex      *coutLock,
                      int               portNumber,
                      int               numConnections,
                      bool              reuseAddressFlag,
                      bslma::Allocator *basicAllocator = 0);
            // Create an echo server that listens for incoming connections on
            // the specified 'portNumber' managing up to the specified
            // 'numConnections' simultaneous connections.  The echo server
            // will use the specified 'coutLock' to synchronize access to the
            // standard output.  Optionally specify a 'basicAllocator' used to
            // supply memory.  If 'basicAllocator' is 0, the currently
            // installed default allocator is used.  The behavior is undefined
            // unless coutLock is not 0.

        ~my_EchoServer();
            // Destroy this server.

        // MANIPULATORS
        const btemt_SessionPool& pool() const;
            // Return a non-modifiable reference to the session pool used by
            // this echo server.

        int portNumber() const;
            // Return the actual port number on which this server is listening.
    };
//..
// Note that this example server prints information depending on
// implicitly-defined static variables and therefore must use a mutex to
// synchronize access to 'bsl::cout'.  A production application should use a
// proper logging mechanism instead such as the 'bael' logger.
//..
    // my_echoserver.h (continued)

                            // -------------------
                            // class my_EchoServer
                            // -------------------

    // PRIVATE MANIPULATORS
    void my_EchoServer::poolStateCb(int reason, int source, void *userData)
    {
        if (veryVerbose) {
            d_coutLock_p->lock();
            bsl::cout << "Pool state changed: (" << reason << ", " << source
                      << ") " << bsl::endl;
            d_coutLock_p->unlock();
        }
    }

    void my_EchoServer::sessionStateCb(int            state,
                                       int            handle,
                                       btemt_Session *session,
                                       void          *userData) {

        switch(state) {
          case btemt_SessionPool::SESSION_DOWN: {
              if (veryVerbose) {
                  d_coutLock_p->lock();
                  bsl::cout << "Client from "
                            << session->channel()->peerAddress()
                            << " has disconnected."
                            << bsl::endl;
                  d_coutLock_p->unlock();
              }
          } break;
          case btemt_SessionPool::SESSION_UP: {
              if (veryVerbose) {
                  d_coutLock_p->lock();
                  bsl::cout << "Client connected from "
                            << session->channel()->peerAddress()
                            << bsl::endl;
                  d_coutLock_p->unlock();
              }
          } break;
        }
    }

    // CREATORS
    my_EchoServer::my_EchoServer(bcemt_Mutex      *lock,
                                 int               portNumber,
                                 int               numConnections,
                                 bool              reuseAddressFlag,
                                 bslma::Allocator *basicAllocator)
    : d_sessionFactory(basicAllocator)
    , d_coutLock_p(lock)
    , d_allocator_p(bslma::Default::allocator(basicAllocator))
    {
        d_config.setMaxThreads(4);                  // 4 I/O threads
        d_config.setMaxConnections(numConnections);
        d_config.setReadTimeout(5.0);               // in seconds
        d_config.setMetricsInterval(10.0);          // seconds
        d_config.setMaxWriteCache(1<<10);           // 1Mb
        d_config.setIncomingMessageSizes(1, 100, 1024);

        typedef btemt_SessionPool::SessionPoolStateCallback SessionPoolStateCb;

        SessionPoolStateCb poolStateCb = bdef_MemFnUtil::memFn(
                                            &my_EchoServer::poolStateCb, this);

        d_sessionPool_p = new (*d_allocator_p)
                             btemt_SessionPool(d_config,
                                               poolStateCb,
                                               basicAllocator);

        btemt_SessionPool::SessionStateCallback sessionStateCb =
                       bdef_MemFnUtil::memFn(&my_EchoServer::sessionStateCb,
                                             this);

        ASSERT(0 == d_sessionPool_p->start());
        int handle;
        ASSERT(0 == d_sessionPool_p->listen(&handle, sessionStateCb,
                                            portNumber,
                                            numConnections,
                                            reuseAddressFlag,
                                            &d_sessionFactory));

        d_portNumber = d_sessionPool_p->portNumber(handle);
    }

    my_EchoServer::~my_EchoServer()
    {
        d_sessionPool_p->stop();
        d_allocator_p->deleteObjectRaw(d_sessionPool_p);
    }

    // ACCESSORS
    const btemt_SessionPool& my_EchoServer::pool() const
    {
        return *d_sessionPool_p;
    }

    int my_EchoServer::portNumber() const
    {
        return d_portNumber;
    }
//..
// We can implement a simple "Hello World!" example to exercise our echo
// server.
//..
    int usageExample(bslma::Allocator *allocator) {

        enum {
            BACKLOG = 5,
            REUSE   = 1
        };

        my_EchoServer echoServer(&coutMutex, 0, BACKLOG, REUSE);

        bteso_InetStreamSocketFactory<bteso_IPv4Address> factory;
        bteso_StreamSocket<bteso_IPv4Address> *socket = factory.allocate();

        const char STRING[] = "Hello World!";

        const bteso_IPv4Address ADDRESS("127.0.0.1", echoServer.portNumber());
        ASSERT(0 == socket->connect(ADDRESS));
        ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

        char readBuffer[sizeof(STRING)];
        ASSERT(sizeof(STRING) == socket->read(readBuffer, sizeof(STRING)));
        ASSERT(0 == bsl::strcmp(readBuffer, STRING));

        factory.deallocate(socket);
        return 0;
    }
//..

}  // close namespace BTEMT_SESSION_POOL_USAGE_EXAMPLE

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? bsl::atoi(argv[1]) : 0;
    verbose = argc > 2;
    veryVerbose = argc > 3;
    veryVeryVerbose = argc > 4;

    bsl::cout << "TEST " << __FILE__ << " CASE " << test << bsl::endl;

    bcema_TestAllocator ta("ta", veryVeryVerbose);

    switch (test) { case 0:  // Zero is always the leading case.
      case 15: {
        // --------------------------------------------------------------------
        // TEST USAGE EXAMPLE
        //   The usage example from the header has been incorporated into this
        //   test driver.  All references to 'assert' have been replaced with
        //   'ASSERT'.  Call the test example function and assert that it works
        //   as expected.
        //
        // Plan:
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "USAGE EXAMPLE" << bsl::endl
                               << "=============" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_USAGE_EXAMPLE;

        usageExample(&ta);
        ASSERT(0 == ta.numBytesInUse());
        ASSERT(0 == ta.numMismatches());

      } break;
      case 14: {
        // --------------------------------------------------------------------
        // TESTING 'allocate' with shared async channel
        //   Ensure that the 'channel' passed to the session objects remains
        //   valid following the session pool's deallocation of it.
        //
        // Concerns:
        //   1. The async channel remains valid for access after the session
        //      pool has deallocated it.
        //
        // Plan:
        //   1. For concern 1, invoke 'allocate' with a shared pointer to the
        //      channel. Coordinate access to the underlying btemt_AsyncChannel
        //      at a later point in time than would have been possible prior to
        //      the addition of the introduction of channel sharing.
        //
        // Testing:
        //   DRQS 44879376
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'allocate' with shared channel"
                               << bsl::endl
                               << "======================================"
                               << bsl::endl;

        using namespace BTEMT_SESSION_POOL_DRQS_44879376;

        bteso_InetStreamSocketFactory<bteso_IPv4Address> factory;
        bteso_StreamSocket<bteso_IPv4Address> *socket = factory.allocate();

        bteso_IPv4Address address("127.0.0.1",
                                  bteso_IPv4Address::BTESO_ANY_PORT);

        bcemt_Barrier barrier(3);

        Tester tester(Tester::LISTENER,
                      &barrier,
                      address,
                      false,
                      &ta);

        int portNumber = tester.portNumber();

        address.setPortNumber(portNumber);

        ASSERT(0 == socket->connect(address));

        const char STRING[] = "Hello World!";

        ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

        char readBuffer[sizeof(STRING)];
        ASSERT(sizeof(STRING) == socket->read(readBuffer, sizeof(STRING)));

        ASSERT(0 == bsl::strcmp(readBuffer, STRING));

        if (veryVerbose) {
            MTCOUT << "Bringing down the channel" << MTENDL;
        }

        socket->shutdown(bteso_Flag::BTESO_SHUTDOWN_BOTH);

        factory.deallocate(socket);

        barrier.wait();

      } break;
      case 13: {
        // --------------------------------------------------------------------
        // TESTING CALLING 'start' AFTER 'stop'
        //   Ensure that 'start' after a 'stop' works as expected.
        //
        // Concerns:
        //: 1. 'start' after a 'stop' restarts the session pool.
        //
        // Plan:
        //: 1 Create a session pool object, mX.
        //:
        //: 2 Open a pre-defined number of listening sockets in mX.
        //:
        //: 3 Create a number of threads that each connect to one of the
        //:   listening sockets in mX.
        //:
        //: 4 Create a number of threads that each listen on a socket.
        //:
        //: 5 Open a session in mX by connecting a listening sockets created
        //:   in step 4.
        //:
        //: 6 Write large amount of data through mX across all the open
        //:   channels.
        //:
        //: 7 Invoke 'stopAndRemoveAllSessions' and confirm that no sessions
        //:   are outstanding.
        //:
        //: 8 Call 'start' and check the return value.
        //:
        //: 9 Repeat steps 2 - 7 to confirm that opening connections and
        //:   transferring data works.
        //:
        //
        // Testing:
        //   int start();
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'start' after 'stop'" << bsl::endl
                               << "============================" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS;

        typedef btemt_SessionPool::SessionStateCallback     SessionCb;
        typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

        btemt_ChannelPoolConfiguration config;
        config.setMaxThreads(2 * NUM_THREADS);
        config.setWriteCacheWatermarks(0, NUM_BYTES * 10);  // 1Mb

        PoolCb    poolStateCb(&poolStateCallback);
        SessionCb sessionStateCb = bdef_BindUtil::bind(
                                &sessionStateCallbackUsingChannelMapAndCounter,
                                _1,
                                _2,
                                _3,
                                _4,
                                &numUpConnections);

        const int                     SIZE = 1024 * 1024; // 1 MB
        bcema_PooledBlobBufferFactory factory(SIZE);
        bcema_Blob                    dataBlob(&factory);
        dataBlob.setLength(NUM_BYTES);

        TestFactory sessionFactory;
        Obj         mX(config, poolStateCb, true);  const Obj& X = mX;

        ASSERT(0 == mX.start());

        bcemt_ThreadUtil::Handle connectThreads[NUM_THREADS];
        bcemt_ThreadUtil::Handle listenThreads[NUM_THREADS];

        runTestFunction(connectThreads,
                        listenThreads,
                        &mX,
                        &sessionStateCb,
                        &sessionFactory,
                        dataBlob);

        ASSERT(0 != mX.numSessions());

        ASSERT(0 == mX.stopAndRemoveAllSessions());

        ASSERT(0 == mX.numSessions());

        ASSERT(0 == mX.start());

        runTestFunction(connectThreads,
                        listenThreads,
                        &mX,
                        &sessionStateCb,
                        &sessionFactory,
                        dataBlob);

        ASSERT(0 != mX.numSessions());

        ASSERT(0 == mX.stopAndRemoveAllSessions());

        ASSERT(0 == mX.numSessions());
      } break;
      case 12: {
        // --------------------------------------------------------------------
        // TESTING 'connect' with socket options
        //   Ensure that the 'connect' that takes a socket option object
        //   returns a session up callback on success and user callback on
        //   error.
        //
        // Concerns:
        //   1. The session state callback is called with success if setting of
        //      the socket options succeeds.
        //
        //   2. A user callback is invoked on encountering an asynchronous
        //      failure while setting the socket options.
        //
        // Plan:
        //   1. For concern 1, invoke 'connect' with a socket option expected
        //      to succeed.  Verify that a session state callback is invoked
        //      informing the user of connect success.
        //
        //   1. For concern 2, invoke 'connect' with a socket option expected
        //      to fail.  Verify that a pool state callback with is invoked
        //      informing the user of the connect error.
        //
        // Testing:
        //   int connect(host, ...., *socketOptions);
        //   int connect(serverAddr, ...., *socketOptions);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'connect' with socket options"
                               << bsl::endl
                               << "====================================="
                               << bsl::endl;

        using namespace BTEMT_SESSION_POOL_SETTING_SOCKETOPTIONS;

        if (verbose) cout << "connect(IPv4Address...) with socket options"
                          << endl;
        {
            btemt_ChannelPoolConfiguration config;
            config.setMaxThreads(1);

            typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

            bcemt_Barrier   channelCbBarrier(2);
            btemt_SessionPool::SessionStateCallback sessionStateCb(
                          bdef_BindUtil::bind(&sessionStateCallbackWithBarrier,
                                              _1, _2, _3, _4,
                                              &channelCbBarrier));

            int             poolState;
            bcemt_Barrier   poolCbBarrier(2);
            btemt_SessionPool::SessionPoolStateCallback
                 poolStateCb(bdef_BindUtil::bind(&poolStateCallbackWithBarrier,
                                                 _1, _2, _3,
                                                 &poolState,
                                                 &poolCbBarrier));

            TestFactory sessionFactory;

            Obj pool(config, poolStateCb, false);

            ASSERT(0 == pool.start());

            bteso_InetStreamSocketFactory<bteso_IPv4Address> socketFactory;

            {

                typedef bteso_StreamSocketFactoryDeleter Deleter;

                bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                    socket(socketFactory.allocate(), 
                           &socketFactory,
                           &Deleter::deleteObject<bteso_IPv4Address>);

                SocketOptions opt;  opt.setKeepAlive(true); // always succeeds 
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                &opt, 0, 0);
                ASSERT(!rc);

                channelCbBarrier.wait();
            }

            {

                typedef bteso_StreamSocketFactoryDeleter Deleter;

                bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                    socket(socketFactory.allocate(), 
                           &socketFactory,
                           &Deleter::deleteObject<bteso_IPv4Address>);
                ASSERT(socket);

                SocketOptions opt;  opt.setSendTimeout(1); // fails on all
                                                           // platforms 
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                &opt, 0, 0);
                ASSERT(!rc);

                poolCbBarrier.wait();
                ASSERT(btemt_SessionPool::CONNECT_FAILED == poolState);
            }

            ASSERT(0 == pool.stopAndRemoveAllSessions());
        }
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // TESTING 'connect' with a user-specified local address
        //   Ensure that the 'connect' that takes a client address returns a
        //   session up callback on success and user callback on error.
        //
        // Concerns:
        //   1. The session state callback is called with success if binding
        //      to the local address succeeds.
        //
        //   2. A user callback is invoked on encountering an asynchronous
        //      failure while binding to a provided local address.
        //
        // Plan:
        //   1. For concern 1, invoke 'connect' with a good IP address.  Verify
        //      that a session state callback is invoked informing the user of
        //      connect success.
        //
        //   2. For concern 2, invoke 'connect' with a bad IP address.  Verify
        //      that a pool state callback is invoked informing the user of
        //      the connect error.
        //
        // Testing:
        //   int connect(host, ...., *socketOptions, *clientAddr);
        //   int connect(serverAddr, ...., *socketOptions, *clientAddr);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'connect' with a local address"
                               << bsl::endl
                               << "======================================"
                               << bsl::endl;

        using namespace BTEMT_SESSION_POOL_SETTING_SOCKETOPTIONS;

        if (verbose) cout << "connect(IPv4Address...) with client address"
                          << endl;
        {
            btemt_ChannelPoolConfiguration config;
            config.setMaxThreads(1);

            typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

            bcemt_Barrier   channelCbBarrier(2);
            btemt_SessionPool::SessionStateCallback sessionStateCb(
                          bdef_BindUtil::bind(&sessionStateCallbackWithBarrier,
                                              _1, _2, _3, _4,
                                              &channelCbBarrier));

            int             poolState;
            bcemt_Barrier   poolCbBarrier(2);
            btemt_SessionPool::SessionPoolStateCallback
                 poolStateCb(bdef_BindUtil::bind(&poolStateCallbackWithBarrier,
                                                 _1, _2, _3,
                                                 &poolState,
                                                 &poolCbBarrier));

            TestFactory sessionFactory;

            Obj pool(config, poolStateCb, false);

            ASSERT(0 == pool.start());

            bteso_InetStreamSocketFactory<bteso_IPv4Address> socketFactory;

            {

                typedef bteso_StreamSocketFactoryDeleter Deleter;

                bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                    socket(socketFactory.allocate(), 
                           &socketFactory,
                           &Deleter::deleteObject<bteso_IPv4Address>);

                bteso_IPv4Address address("127.0.0.1", 45000); // good address
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                0, &socketFactory,
                                                &address);
                ASSERT(!rc);

                if (veryVerbose) {
                    MTCOUT << "Waiting on channel barrier..." << MTENDL;
                }
                channelCbBarrier.wait();
            }

            {

                typedef bteso_StreamSocketFactoryDeleter Deleter;

                bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                    socket(socketFactory.allocate(), 
                           &socketFactory,
                           &Deleter::deleteObject<bteso_IPv4Address>);

                bteso_IPv4Address address("1.1.1.1", 45000);  // bad address
                const int rc = createConnection(&pool,
                                                &sessionStateCb,
                                                &sessionFactory,
                                                socket.ptr(),
                                                0, &socketFactory,
                                                &address);
                if (!rc) {
                    if (veryVerbose) {
                        MTCOUT << "Waiting on pool barrier..." << MTENDL;
                    }
                    poolCbBarrier.wait();
                    ASSERT(btemt_SessionPool::CONNECT_FAILED == poolState);
                }
            }

            ASSERT(0 == pool.stopAndRemoveAllSessions());
        }
      } break;
      case 10: {
        // --------------------------------------------------------------------
        // TESTING 'setWriteCacheWatermarks'
        //   The 'setWriteCacheWatermarks' method has the expected effect.
        //
        // Concerns:
        //   1. That 'setWriteCacheWatermarks' fails when passed an invalid
        //      session id.
        //
        //   2. That 'setWriteCacheWatermarks' correctly passes its arguments
        //      to btemt_ChannelPool::setWriteCacheWatermarks'.
        //
        // Plan:
        //   1. For concern 1, invoke 'setWriteCacheWatermarks' with an invalid
        //      session id and verify that the method fails.
        //
        //   2. For concern 2, create a session, capturing the session id of a
        //      client connection.  Invoke the method using that session id and
        //      representative (valid) values for the low- and high-water
        //      marks.  Assert that the method succeeds in each case.
        //
        // Testing:
        //   int setWriteCacheWatermarks(int, int, int);
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'setWriteCacheWatermarks'"
                               << bsl::endl
                               << "================================="
                               << bsl::endl;

        using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        enum {
            LOW_WATERMARK =  512,
            HI_WATERMARK  = 4096
        };

        typedef btemt_SessionPool::SessionStateCallback     SessionCb;
        typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

        btemt_ChannelPoolConfiguration config;
        config.setMaxThreads(4);
        config.setWriteCacheWatermarks(LOW_WATERMARK, HI_WATERMARK);

        bsls::AtomicInt numUpConnections(0);

        PoolCb poolStateCb(&poolStateCallback);
        SessionCb sessionStateCb = bdef_BindUtil::bind(
                                              &sessionStateCallbackWithCounter,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &numUpConnections);

        Obj sessionPool(config, poolStateCb, false);

        ASSERT(0 == sessionPool.start());

        TestFactory factory;

        int handle = 0;
        ASSERT(0 == sessionPool.listen(&handle,
                                       sessionStateCb,
                                       0,
                                       5,
                                       1,
                                       &factory));
        const int PORTNUM = sessionPool.portNumber(handle);

        bteso_IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        ASSERT(0 == sessionPool.connect(&handle,
                                        sessionStateCb,
                                        ADDRESS,
                                        5,
                                        bdet_TimeInterval(1),
                                        &factory));

        while (numUpConnections < 2) {
            bcemt_ThreadUtil::microSleep(50, 0);
        }

        ASSERT(0 != sessionPool.setWriteCacheWatermarks(handle + 666,
                                                        LOW_WATERMARK + 1,
                                                        HI_WATERMARK - 1));

        ASSERT(0 == sessionPool.setWriteCacheWatermarks(handle,
                                                     LOW_WATERMARK,
                                                     HI_WATERMARK));
        ASSERT(0 == sessionPool.setWriteCacheWatermarks(handle,
                                                     LOW_WATERMARK,
                                                     LOW_WATERMARK));

        sessionPool.setWriteCacheWatermarks(handle,
                                            LOW_WATERMARK, HI_WATERMARK);
        ASSERT(0 == sessionPool.setWriteCacheWatermarks(handle,
                                                        HI_WATERMARK,
                                                        HI_WATERMARK));

        sessionPool.setWriteCacheWatermarks(handle,
                                            LOW_WATERMARK, HI_WATERMARK);
        ASSERT(0 == sessionPool.setWriteCacheWatermarks(handle,
                                                        LOW_WATERMARK - 2,
                                                        LOW_WATERMARK));

        sessionPool.setWriteCacheWatermarks(handle,
                                            LOW_WATERMARK, HI_WATERMARK);
        ASSERT(0 == sessionPool.setWriteCacheWatermarks(handle,
                                                        LOW_WATERMARK - 2,
                                                        LOW_WATERMARK - 1));

        sessionPool.setWriteCacheWatermarks(handle,
                                            LOW_WATERMARK, HI_WATERMARK);
        ASSERT(0 == sessionPool.setWriteCacheWatermarks(handle,
                                                        HI_WATERMARK,
                                                        HI_WATERMARK + 2));

        sessionPool.setWriteCacheWatermarks(handle,
                                            LOW_WATERMARK, HI_WATERMARK);
        ASSERT(0 == sessionPool.setWriteCacheWatermarks(handle,
                                                        HI_WATERMARK + 1,
                                                        HI_WATERMARK + 2));

        ASSERT(0 == sessionPool.stop());
      } break;
      case 9: {
        // --------------------------------------------------------------------
        // TESTING 'stopAndRemoveAllSessions'
        //
        // Concerns
        //: 1 Session pool stops all threads and removes all sessions when
        //:   this function is called.
        //:
        //: 2 Any resources associated with any session is released.
        //
        // Plan:
        //: 1 Create a session pool object, mX.
        //:
        //: 2 Open a pre-defined number of listening sockets in mX.
        //:
        //: 3 Create a number of threads that each connect to one of the
        //:   listening sockets in mX.
        //:
        //: 4 Create a number of threads that each listen on a socket.
        //:
        //: 5 Open a session in mX by connecting a listening sockets created
        //:   in step 4.
        //:
        //: 6 Write large amount of data through mX across all the open
        //:   channels.
        //:
        //: 7 Invoke 'stopAndRemoveAllSessions' and confirm that no sessions
        //:   are outstanding.
        //
        // Testing:
        //   int stopAndRemoveAllSessions();
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "TESTING 'stopAndRemoveAllChannels'"
                               << bsl::endl
                               << "=================================="
                               << bsl::endl;

        using namespace BTEMT_SESSION_POOL_STOPANDREMOVEALLSESSIONS;

        typedef btemt_SessionPool::SessionStateCallback SessionCb;
        typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

        btemt_ChannelPoolConfiguration config;
        config.setMaxThreads(2 * NUM_THREADS);
        config.setWriteCacheWatermarks(0, NUM_BYTES * 10);  // 1Mb

        PoolCb poolStateCb(&poolStateCallback);
        SessionCb sessionStateCb = bdef_BindUtil::bind(
                                &sessionStateCallbackUsingChannelMapAndCounter,
                                _1,
                                _2,
                                _3,
                                _4,
                                &numUpConnections);

        TestFactory sessionFactory;
        Obj         mX(config, poolStateCb, true);  const Obj& X = mX;

        ASSERT(0 == mX.start());

        const int                     SIZE = 1024 * 1024; // 1 MB
        bcema_PooledBlobBufferFactory factory(SIZE);
        bcema_Blob                    dataBlob(&factory);
        dataBlob.setLength(NUM_BYTES);

        bcemt_ThreadUtil::Handle connectThreads[NUM_THREADS];
        bcemt_ThreadUtil::Handle listenThreads[NUM_THREADS];

        memset(connectThreads, 0, sizeof(connectThreads));
        memset(listenThreads, 0, sizeof(listenThreads));

        runTestFunction(connectThreads,
                        listenThreads,
                        &mX,
                        &sessionStateCb,
                        &sessionFactory,
                        dataBlob);

        ASSERT(0 != mX.numSessions());

        const int rc = mX.stopAndRemoveAllSessions();
        ASSERT(0 == rc);

        ASSERT(0 == mX.numSessions());

        for (int i = 0; i < NUM_THREADS; ++i) {
            bcemt_ThreadUtil::join(connectThreads[i]);
            bcemt_ThreadUtil::join(listenThreads[i]);
        }
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // REPRODUCING DRQS 28731692
        //  Ensure that session pool owns the factories used for allocating
        //  read buffers.
        //
        // Concerns
        //: 1 Session pool owns the factories that are used to allocate the
        //:   buffers provided to clients in the data callback.
        //
        // Plan:
        //: 1 Create a bcema_Blob that stores the data returned in the data
        //:   callback.
        //:
        //: 2 Create a session pool object, 'mX', designed to use pooled buffer
        //:   chains and listen on a port on the local machine.
        //:
        //: 3 Create a socket and 'connect' to the port number on which the
        //:   session pool is listening.
        //:
        //: 4 Write data on the socket.
        //:
        //: 5 Destroy the session pool.
        //:
        //: 6 Destroy the blob.
        //
        // Testing:
        //  DRQS 28731692
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "DRQS 28731692" << bsl::endl
                               << "=============" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        {
            btemt_ChannelPoolConfiguration config;
            config.setMaxThreads(4);

            typedef btemt_SessionPool::SessionStateCallback     SessionCb;
            typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

            PoolCb    poolCb    = &poolStateCallback;
            SessionCb sessionCb = &sessionStateCallback;

            bcema_Blob           blob;
            bslma::TestAllocator ta1, ta2;
            BlobReadCallback     callback(bdef_BindUtil::bind(&readCbWithBlob,
                                                              _1,
                                                              _2,
                                                              _3,
                                                              _4,
                                                              &blob));

            TestFactory factory(true, &callback, &ta2);
            Obj         mX(config, poolCb, false, &ta1);  const Obj& X = mX;

            ASSERT(0 == mX.start());

            int handle = 0;
            ASSERT(0 == mX.listen(&handle,
                                  sessionCb,
                                  0,
                                  5,
                                  &factory));
            const int PORTNUM = mX.portNumber(handle);
            {
                bteso_InetStreamSocketFactory<bteso_IPv4Address> factory;
                bteso_StreamSocket<bteso_IPv4Address> *socket =
                                                            factory.allocate();
                bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                   smp(socket,
                       &factory,
                       &SocketFactoryDeleter::deleteObject<bteso_IPv4Address>);

                const bteso_IPv4Address ADDRESS("127.0.0.1", PORTNUM);
                int rc = socket->connect(ADDRESS);
                ASSERT(!rc);

                const int PAYLOAD_SIZE = 320;
                char payload[PAYLOAD_SIZE];
                bsl::memset(payload, 'Z', PAYLOAD_SIZE);
                socket->write(payload, PAYLOAD_SIZE);
            }
        }
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // REPRODUCING DRQS 29067989
        //  Ensure that session pool does not allocate and hold on to
        //  additional memory buffers when clients read all the data.
        //
        // Concerns
        //: 1 Session pool does not allocate and hold on to memory buffers
        //:   indefinitely even if the client is reading all the data.
        //
        // Plan:
        //: 1 Create a session pool object, 'mX', and listen on a port on the
        //:   local machine.
        //:
        //: 2 Create a socket and 'connect' to the port number on which the
        //:   session pool is listening.
        //:
        //: 3 Write data on the socket in multiple attempts and monitor the
        //:   size and length of the blob returned by session pool in the data
        //:   callback.
        //:
        //: 4 Verify that the blob provided in the data callback does not
        //:   unnecessarily hoard memory.
        //
        // Testing:
        //  DRQS 29067989
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "DRQS 29067989" << bsl::endl
                               << "=============" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_DRQS_29067989;

        {
            btemt_ChannelPoolConfiguration config;
            config.setMaxThreads(4);

            char payload[PAYLOAD_SIZE];
            bsl::memset(payload, 'X', PAYLOAD_SIZE);
            const int NUM_WRITES = 10000;

            bslma::TestAllocator ta;
            bcemt_Semaphore      semaphore;
            BlobReadCallback     callback =
                                        bdef_BindUtil::bind(
                                                     &readCbWithMetrics,
                                                     _1,
                                                     _2,
                                                     _3,
                                                     _4,
                                                     NUM_WRITES * PAYLOAD_SIZE,
                                                     &semaphore);

            TestFactory sessionFactory(true, &callback, &ta);
            Obj         mX(config, &poolStateCallback, true, &ta);
            const Obj&  X = mX;

            ASSERT(0 == mX.start());

            int handle = 0;
            ASSERT(0 == mX.listen(&handle,
                                  &sessionStateCallback,
                                  0,
                                  5,
                                  &sessionFactory));

            const int PORTNUM = mX.portNumber(handle);

            bteso_InetStreamSocketFactory<bteso_IPv4Address> factory;
            bteso_StreamSocket<bteso_IPv4Address> *socket = factory.allocate();
            bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                   smp(socket,
                       &factory,
                       &SocketFactoryDeleter::deleteObject<bteso_IPv4Address>);

            const bteso_IPv4Address ADDRESS("127.0.0.1", PORTNUM);
            int rc = socket->connect(ADDRESS);
            ASSERT(!rc);

            for (int i = 0; i < NUM_WRITES; ++i) {
                socket->write(payload, PAYLOAD_SIZE);
            }

            semaphore.wait();

            if (veryVerbose) {
                MTCOUT << "TA In Use: " << testAllocator.numBytesInUse()
                       << MTENDL;
                MTCOUT << "TA In Use Blocks: "
                       << testAllocator.numBlocksInUse()
                       << MTENDL;
                MTCOUT << "TA In Max: " << testAllocator.numBytesMax()
                       << MTENDL;
                MTCOUT << "maxLength: " << maxLength << MTENDL;
                MTCOUT << "maxSize: " << maxSize << MTENDL;
                MTCOUT << "maxExtra: " << maxExtra << MTENDL;
                MTCOUT << "maxNumBuffers: " << maxNumBuffers << MTENDL;
                MTCOUT << "maxNumDataBuffers: " << maxNumDataBuffers << MTENDL;
            }
        }
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // REPRODUCING DRQS 24968477
        //  Ensure that the bug where d_numSessions is decremented in stop and
        //  then again when the session handle is destroyed has been fixed.
        //
        // Concerns:
        //: 1 d_numSessions is not decremented twice after a call to 'stop'.
        //
        // Plan:
        //: 1 Create a session pool object, 'mX', and listen on a port on the
        //:   local machine.
        //:
        //: 2 Define NUM_SESSIONS with a value of 2 as the number of sessions
        //:   to be created.
        //:
        //: 3 Open NUM_SESSIONS sockets and 'connect' to the port number on
        //:   which the session pool is listening.
        //:
        //: 4 Confirm that numSessions on mX returns NUM_SESSIONS.
        //:
        //: 5 Call 'stop' on the session pool and confirm that numSessions
        //:   returns 0.
        //
        // Testing:
        //  DRQS 24968477
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "DRQS 24968477" << bsl::endl
                               << "=============" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        typedef btemt_SessionPool::SessionStateCallback     SessionCb;
        typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

        const int NUM_SESSIONS = 5;
        btemt_ChannelPoolConfiguration config;
        config.setMaxThreads(NUM_SESSIONS);
        config.setMaxConnections(NUM_SESSIONS);

        PoolCb          poolStateCb(&poolStateCallback);
        bcemt_Barrier   barrier(NUM_SESSIONS + 1);
        bsls::AtomicInt numUpConnections(0);

        SessionCb sessionStateCb(bdef_BindUtil::bind(
                                              &sessionStateCallbackWithCounter,
                                              _1, _2, _3, _4,
                                              &numUpConnections));

        Obj mX(config, poolStateCb, false);  const Obj& X = mX;

        ASSERT(0 == mX.start());

        int         handle;
        TestFactory sessionFactory;
        int rc = mX.listen(&handle, sessionStateCb, 0, 5, &sessionFactory);
        ASSERT(!rc);

        const int PORTNUM = X.portNumber(handle);

        bteso_IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        bteso_InetStreamSocketFactory<bteso_IPv4Address> socketFactory;
        bsl::vector<bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> > >
                                                         sockets(NUM_SESSIONS);
        for (int i = 0; i < NUM_SESSIONS; ++i) {
            bteso_StreamSocket<bteso_IPv4Address> *socket =
                                                      socketFactory.allocate();

            ASSERT(0 == socket->connect(ADDRESS));

            bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                   smp(socket,
                       &socketFactory,
                       &SocketFactoryDeleter::deleteObject<bteso_IPv4Address>);
            sockets[i] = smp;
        }

        while (numUpConnections < NUM_SESSIONS) {
            bcemt_ThreadUtil::microSleep(50, 0);
        }

        int ns = X.numSessions();
        LOOP_ASSERT(ns, NUM_SESSIONS == ns);

        ASSERT(0 == mX.stop());

        ns = X.numSessions();
        LOOP_ASSERT(ns, 0 == ns);
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // REPRODUCING DRQS 20535695
        //
        // Plan:
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "DRQS 20535695" << bsl::endl
                               << "=============" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_DRQS_20535695;

        typedef btemt_SessionPool::SessionStateCallback SessionCb;
        typedef btemt_SessionPool::SessionPoolStateCallback PoolCb;

        btemt_ChannelPoolConfiguration config;
        config.setMaxThreads(1);

        PoolCb    poolStateCb    = &poolStateCallback;
        SessionCb sessionStateCb = &sessionStateCallback;

        Obj mX(config, poolStateCb, false);  const Obj& X = mX;

        ASSERT(0 == mX.start());

        bcemt_Barrier barrier(2);
        int           handle;
        TestSessionFactory sessionFactory(&barrier);
        mX.listen(&handle, sessionStateCb, 0, 1, &sessionFactory);

        bteso_InetStreamSocketFactory<bteso_IPv4Address> socketFactory;
        bteso_StreamSocket<bteso_IPv4Address> *socket =
                                                      socketFactory.allocate();
        bdema_ManagedPtr<bteso_StreamSocket<bteso_IPv4Address> >
                   smp(socket,
                       &socketFactory,
                       &SocketFactoryDeleter::deleteObject<bteso_IPv4Address>);

        const int PORTNUM = mX.portNumber(handle);

        bteso_IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        ASSERT(0 == socket->connect(ADDRESS));

        socket->shutdown(bteso_Flag::BTESO_SHUTDOWN_BOTH);

        barrier.wait();
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // REPRODUCING DRQS 22373213
        //
        // Plan:
        //
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "DRQS 22373213" << bsl::endl
                               << "=============" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        btemt_ChannelPoolConfiguration config;
        config.setMaxThreads(4);

        int           cbCount = 0;
        bcemt_Barrier barrier(2);

        BlobReadCallback callback = bdef_BindUtil::bind(
                                                    &readCbWithCountAndBarrier,
                                                    _1,
                                                    _2,
                                                    _3,
                                                    _4,
                                                    &cbCount,
                                                    &barrier);
        TestFactory factory(true, &callback);

        typedef btemt_SessionPool::SessionPoolStateCallback SessionPoolStateCb;
        typedef btemt_SessionPool::SessionStateCallback     SessionStateCb;

        SessionPoolStateCb poolCb         = &poolStateCallback;
        SessionStateCb     sessionStateCb = bdef_BindUtil::bind(
                                              &sessionStateCallbackWithBarrier,
                                              _1,
                                              _2,
                                              _3,
                                              _4,
                                              &barrier);

        Obj mX(config, poolCb, false);  const Obj& X = mX;

        int rc = mX.start();
        ASSERT(0 == rc);

        int handle;
        rc = mX.listen(&handle,
                       sessionStateCb,
                       0,
                       5,
                       1,
                       &factory);
        ASSERT(0 == rc);

        const int PORTNUM = mX.portNumber(handle);

        const char STRING[] = "Hello World!";

        bteso_IPv4Address ADDRESS("127.0.0.1", PORTNUM);

        bteso_InetStreamSocketFactory<bteso_IPv4Address> socketFactory;
        bteso_StreamSocket<bteso_IPv4Address> *socket =
                                                      socketFactory.allocate();

        rc = socket->connect(ADDRESS);
        ASSERT(0 == rc);

        barrier.wait();

        rc = socket->write(STRING, sizeof(STRING));
        ASSERT(sizeof(STRING) == rc);

        barrier.wait();

        socketFactory.deallocate(socket);

        LOOP_ASSERT(cbCount, 1 == cbCount);
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // BLOB BASED EXAMPLE
        //
        // Plan:
        // Testing:
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "BLOB BASED EXAMPLE" << bsl::endl
                               << "==================" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        bcema_TestAllocator da("default", veryVeryVerbose);
        bslma::DefaultAllocatorGuard defaultAllocGuard(&da);

        TestFactory factory(true, 0, &ta);

        btemt_ChannelPoolConfiguration config;

        typedef btemt_SessionPool::SessionPoolStateCallback SessionPoolStateCb;
        typedef btemt_SessionPool::SessionStateCallback     SessionStateCb;

        SessionPoolStateCb poolStateCb    = &poolStateCallback;
        SessionStateCb     sessionStateCb = &sessionStateCallback;

        Obj sessionPool(config, poolStateCb, true, &ta);
        int rc = sessionPool.start();
        ASSERT(0 == rc);

        int handle;
        rc = sessionPool.listen(&handle,
                                sessionStateCb,
                                0,
                                5,
                                false,
                                &factory);
        ASSERT(0 == rc);

        {
            bteso_InetStreamSocketFactory<bteso_IPv4Address> factory(&ta);
            bteso_StreamSocket<bteso_IPv4Address> *socket = factory.allocate();

            const char STRING[] = "Hello World!";

            const bteso_IPv4Address ADDRESS("127.0.0.1",
                                            sessionPool.portNumber(handle));
            ASSERT(0 == socket->connect(ADDRESS));
            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            char readBuffer[sizeof(STRING)];
            ASSERT(sizeof(STRING) == socket->read(readBuffer, sizeof(STRING)));
            ASSERT(0 == bsl::strcmp(readBuffer, STRING));

            factory.deallocate(socket);
        }

        ASSERT(0 != ta.numBytesInUse());
        const int NUM_BYTES = da.numBytesInUse();
        LOOP_ASSERT(NUM_BYTES, 0 == NUM_BYTES);
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TEST ALLOCATOR PROPAGATION
        //
        // Plan: Install a default allocator, then execute the substance of
        // the usage example, keeping the SessionPool object in-scope after
        // the test is complete.  Then verify that no memory is outstanding
        // through the default allocator.
        // --------------------------------------------------------------------

        if (verbose) bsl::cout << "ALLOCATOR PROPAGATION" << bsl::endl
                               << "=====================" << bsl::endl;

        using namespace BTEMT_SESSION_POOL_GENERIC_TEST_NAMESPACE;

        bcema_TestAllocator da("default", veryVeryVerbose);
        bslma::DefaultAllocatorGuard defaultAllocGuard(&da);

        TestFactory factory(false, 0, &ta);

        btemt_ChannelPoolConfiguration config;

        typedef btemt_SessionPool::SessionPoolStateCallback SessionPoolStateCb;
        typedef btemt_SessionPool::SessionStateCallback     SessionStateCb;

        SessionPoolStateCb poolStateCb    = &poolStateCallback;
        SessionStateCb     sessionStateCb = &sessionStateCallback;

        Obj mX(config, poolStateCb, &ta);  const Obj& X = mX;
        int rc = mX.start();
        ASSERT(0 == rc);

        int handle;
        rc = mX.listen(&handle,
                       sessionStateCb,
                       0,
                       5,
                       false,
                       &factory);
        ASSERT(0 == rc);

        {
            bteso_InetStreamSocketFactory<bteso_IPv4Address> factory(&ta);
            bteso_StreamSocket<bteso_IPv4Address> *socket = factory.allocate();

            const char STRING[] = "Hello World!";

            const bteso_IPv4Address ADDRESS("127.0.0.1",
                                            mX.portNumber(handle));
            ASSERT(0 == socket->connect(ADDRESS));
            ASSERT(sizeof(STRING) == socket->write(STRING, sizeof(STRING)));

            char readBuffer[sizeof(STRING)];
            ASSERT(sizeof(STRING) == socket->read(readBuffer, sizeof(STRING)));
            ASSERT(0 == bsl::strcmp(readBuffer, STRING));

            factory.deallocate(socket);
        }
        ASSERT(0 != ta.numBytesInUse());
        const int NUM_BYTES = da.numBytesInUse();
        LOOP_ASSERT(NUM_BYTES, 0 == NUM_BYTES);
      } break;

      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST:
        //
        // Plan:
        //
        // Testing:
        //   This "test" *exercises* basic functionality, but *tests* nothing.
        // --------------------------------------------------------------------

        btemt_ChannelPoolConfiguration config;

        Obj X(config, btemt_SessionPool::SessionPoolStateCallback());
      } break;
      default: {
        bsl::cerr << "WARNING: CASE `" << test << "' NOT FOUND." << bsl::endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        bsl::cerr << "Error, non-zero test status = " << testStatus << "."
                  << bsl::endl;
    }
    return testStatus;
}

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2006
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
