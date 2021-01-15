// Couchbase Lite main module
//
// Copyright (c) 2020 Couchbase, Inc All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//#![allow(unused_imports)]
//#![allow(dead_code)]

#[macro_use] extern crate enum_primitive;

pub mod blob;
pub mod database;
pub mod document;
pub mod error;
pub mod fleece;
pub mod fleece_mutable;
pub mod logging;
pub mod query;
pub mod replicator;

mod slice;
mod c_api;

mod fleece_tests;

use self::c_api::*;


//////// RE-EXPORT:


pub use blob::*;
pub use database::*;
pub use document::*;
pub use error::*;
pub use fleece::*;
pub use fleece_mutable::*;
pub use query::*;
pub use replicator::*;


//////// TOP-LEVEL TYPES:


/// A time value for document expiration. Defined as milliseconds since the Unix epoch (1/1/1970.)
pub struct Timestamp(i64);


/// An opaque token representing a registered listener.
/// When this object is dropped, the listener function will not be called again.
pub struct ListenerToken {
    _ref: *mut CBLListenerToken
}


impl Drop for ListenerToken {
    fn drop(&mut self) {
        unsafe { CBLListener_Remove(self._ref) }
    }
}


//////// MISC. API FUNCTIONS


/** Returns the total number of Couchbase Lite objects. Useful for leak checking. */
pub fn instance_count() -> usize {
    unsafe { return CBL_InstanceCount() as usize }
}

/** Logs the class and address of each Couchbase Lite object. Useful for leak checking.
    @note  May only be functional in debug builds of Couchbase Lite. */
pub fn dump_instances() {
    unsafe { CBL_DumpInstances() }
}


//////// REFCOUNT SUPPORT (INTERNAL)


pub(crate) unsafe fn retain<T>(cbl_ref: *mut T) -> *mut T {
    return CBL_Retain(cbl_ref as *mut CBLRefCounted) as *mut T
}

pub(crate) unsafe fn release<T>(cbl_ref: *mut T) {
    CBL_Release(cbl_ref as *mut CBLRefCounted)
}
