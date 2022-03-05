/*
 * Copyright (c) 2020, 2022, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "jfr/jni/jfrJavaSupport.hpp"
#include "jfr/recorder/checkpoint/types/jfrTypeManager.hpp"
#include "jfr/support/jfrIntrinsics.hpp"
#include "jfr/writers/jfrJavaEventWriter.hpp"
#include "runtime/interfaceSupport.inline.hpp"
#include "utilities/macros.hpp"

static jobject event_writer(JavaThread* t) {
  assert(t != NULL, "invariant");
  DEBUG_ONLY(JfrJavaSupport::check_java_thread_in_java(t);)
  assert(t->has_last_Java_frame(), "invariant");
  // can safepoint here
  ThreadInVMfromJava transition(t);
  return JfrJavaEventWriter::event_writer(t);
}

void* JfrIntrinsicSupport::get_event_writer(JavaThread* t) {
  return event_writer(t);
}

void JfrIntrinsicSupport::write_checkpoint(JavaThread* t) {
  assert(t != nullptr, "invariant");
  assert(JfrThreadLocal::is_vthread(t), "invariant");
  event_writer(t);
}
