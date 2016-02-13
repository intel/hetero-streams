/*
 * Hetero Streams Library - A streaming library for heterogeneous platforms
 * Copyright (c) 2014 - 2016, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 */

#ifndef HSTREAMS_REFCOUNTDESTROYED_H
#define HSTREAMS_REFCOUNTDESTROYED_H

#include <stdint.h>

/// When refcount goes to zero, the object will commit a suicide through "delete this".
/// Always crate these object through new. XXX provide a static "factory" method and make the
/// constructor private/protected?
///
/// Newly created objects start out with refcount = 1 upon creation.
///
/// Default constructor is protected to make the parent classes implement a static create()
/// method which would force usage of new to allocate the space for the object, since destruction
/// in detach() occurs through delete.
///
/// Copy-construction is disallowed. Assignment operator is disallowed.
///
/// Destructor is made virtual. Actually, destructor could check if refcount_ is zero and emit a message if it's not.
/// TODO investigate that idea.
class hStreams_RefCountDestroyed
{
public:
    /// Increases the object's refcount
    void attach();
    /// Decrease the refcount; if it reaches zero, delete this.
    void detach();
protected:
    hStreams_RefCountDestroyed();
    virtual ~hStreams_RefCountDestroyed();
private:
    int64_t refcount_;
    hStreams_RefCountDestroyed(hStreams_RefCountDestroyed const &other);
    hStreams_RefCountDestroyed &operator=(hStreams_RefCountDestroyed const &other);
};


#endif /* HSTREAMS_REFCOUNTDESTROYED_H */
