// bdesu_filedescriptorguard.h                                        -*-C++-*-
#ifndef INCLUDED_BDESU_FILEDESCIPTORGUARD
#define INCLUDED_BDESU_FILEDESCIPTORGUARD

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a RAII guard class used to close files.
//
//@CLASSES:
//  bdesu_FileDescriptorGuard: RAII guard class used to close files
//
//@SEE_ALSO: bdesu_filesystemutil
//
//@AUTHOR: Bill Chapman (bchapman2)
//
//@DESCRIPTION: This component defines a class, 'bdesu_FileDescriptorGuard', an
// object of which manages an open file descriptor, and closes it when the
// guard goes out of scope and is destroyed.  A 'release' method is provided,
// which will release the descriptor from management by the guard.  When a
// released guard is destroyed, nothing happens.  A 'closeAndRelease' method
// is also provided, which closes the managed file handle and puts the guard
// into a released state.
//
///Usage
///-----
//
///Example 1: Close a File Descriptor
/// - - - - - - - - - - - - - - - - -
// Suppose we want to open a file and perform some I/O operations.  We use an
// object of type 'bdesu_FileDescriptorGuard' to ensure this handle is closed
// after the operations are complete.
//
// First, we create a name for our temporary file name and a few local
// varriables.
//..
//  const bsl::string fileName = "essay.txt";
//  int rc;
//..
// Then, we open the file:
//..
//  Util::FileDescriptor fd = Util::open(fileName,
//                                       Util::e_CREATE,
//                                       Util::e_READ_WRITE);
//  assert(Util::k_INVALID_FD != fd);
//..
// Next, we enter a lexical scope and create a guard object to manage 'fd':
//..
//  {
//      bdesu_FileDescriptorGuard guard(fd);
//..
// Then, we declare an essay we would like to write to the file:
//..
//      const char essay[] = {
//                         "If you can't annoy somebody, there is little\n"
//                         "point in writing.\n"
//                         "                Kingsley Amis\n"
//                         "\n"
//                         "It takes a lifetime to build a reputation, and\n"
//                         "five minutes to lose it.\n"
//                         "                Warren Buffet\n"
//                         "\n"
//                         "Originality is stubborn but not indestructible.\n"
//                         "You can't tell it what to do, and if you try too\n"
//                         "hard to steer it, you either chase it away or\n"
//                         "murder it.\n"
//                         "                Salman Khan\n" };
//..
// Next, we write our essay to the file:
//..
//      rc = Util::write(fd, essay, sizeof(essay));
//      assert(sizeof(essay) == rc);
//..
// Now, 'guard' goes out of scope, and its destructor closes the file
// descriptor.
//..
//  }
//..
// Finally, we observe that further attempts to access 'fd' fail because the
// descriptor has been closed:
//..
//  Util::Offset off = Util::seek(fd,
//                                0,
//                                Util::e_SEEK_FROM_BEGINNING);
//  assert(-1 == off);
//..

#ifndef INCLUDED_BDESCM_VERSION
#include <bdescm_version.h>
#endif

#ifndef INCLUDED_BDESU_FILESYSTEMUTIL
#include <bdesu_filesystemutil.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

namespace BloombergLP {

                     // ================================
                     // struct bdesu_FileDescriptorGuard
                     // ================================

struct bdesu_FileDescriptorGuard {
    // This class implements a guard that conditionally closes an open file
    // descriptor upon its destruction.

    // DATA
    bdesu_FilesystemUtil::FileDescriptor d_descriptor;    // handle for the
                                                          // merged file

  private:
    // NOT IMPLEMENTED
    bdesu_FileDescriptorGuard(const bdesu_FileDescriptorGuard&);
    bdesu_FileDescriptorGuard& operator=(const bdesu_FileDescriptorGuard&);

  public:
    // CREATORS
    explicit
    bdesu_FileDescriptorGuard(bdesu_FilesystemUtil::FileDescriptor descriptor);
        // Create a guard object that will manage the specified 'descriptor',
        // closing it upon destruction (unless either 'realease' or
        // 'closeAndRelease' has been called).  It is permissible for
        // 'descriptor' to be 'bdesu_FilesystemUtil::k_INVALID_FD', in which
        // case the guard created will not manage anything.

    ~bdesu_FileDescriptorGuard();
        // If this guard object manages a file, close that file.

    // MANIPULATORS
    void closeAndRelease();
        // Close the file managed by this guard and release that file from
        // management by this object.  The behavior is undefined unless this
        // object is managing a file.

    bdesu_FilesystemUtil::FileDescriptor release();
        // Release the file from management by this object (without closing
        // it) and return the formerly managed descriptor.  The behavior is
        // undefined unless this object is managing a file.

    // ACCESSORS
    bdesu_FilesystemUtil::FileDescriptor descriptor() const;
        // If this guard is managing a file, return the file descriptor
        // referring to that file, and return
        // 'bdesu_FileSystemUtil::INVALID_FD' otherwise.
};

// ============================================================================
//                          INLINE FUNCTION DEFINITIONS
// ============================================================================

// CREATORS
inline
bdesu_FileDescriptorGuard::bdesu_FileDescriptorGuard(
                               bdesu_FilesystemUtil::FileDescriptor descriptor)
: d_descriptor(descriptor)
{
}

inline
bdesu_FileDescriptorGuard::~bdesu_FileDescriptorGuard()
{
    if (bdesu_FilesystemUtil::k_INVALID_FD != d_descriptor) {
        closeAndRelease();
    }
}

// MANIPULATORS
inline
bdesu_FilesystemUtil::FileDescriptor bdesu_FileDescriptorGuard::release()
{
    BSLS_ASSERT(bdesu_FilesystemUtil::k_INVALID_FD != d_descriptor);

    bdesu_FilesystemUtil::FileDescriptor ret = d_descriptor;
    d_descriptor = bdesu_FilesystemUtil::k_INVALID_FD;

    return ret;
}

// ACCESSORS
inline
bdesu_FilesystemUtil::FileDescriptor bdesu_FileDescriptorGuard::descriptor()
                                                                          const
{
    return d_descriptor;
}

}  // close namespace BloombergLP

#endif

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2013
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ------------------------------ END-OF-FILE ---------------------------------
