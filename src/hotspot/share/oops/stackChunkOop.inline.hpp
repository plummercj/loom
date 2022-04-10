/*
 * Copyright (c) 2021, 2022, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_OOPS_STACKCHUNKOOP_INLINE_HPP
#define SHARE_OOPS_STACKCHUNKOOP_INLINE_HPP

#include "oops/stackChunkOop.hpp"

#include "gc/shared/collectedHeap.hpp"
#include "memory/memRegion.hpp"
#include "memory/universe.hpp"
#include "oops/instanceStackChunkKlass.inline.hpp"
#include "runtime/frame.inline.hpp"
#include "runtime/handles.inline.hpp"
#include "runtime/registerMap.hpp"
#include "runtime/smallRegisterMap.inline.hpp"
#include "utilities/macros.hpp"
#include CPU_HEADER_INLINE(stackChunkOop)

DEF_HANDLE_CONSTR(stackChunk, is_stackChunk_noinline)

inline stackChunkOop stackChunkOopDesc::cast(oop obj) {
  assert(obj == nullptr || obj->is_stackChunk(), "Wrong type");
  return stackChunkOop(obj);
}

inline stackChunkOop stackChunkOopDesc::parent() const         { return stackChunkOopDesc::cast(jdk_internal_vm_StackChunk::parent(as_oop())); }
template<typename P>
inline bool stackChunkOopDesc::is_parent_null() const          { return jdk_internal_vm_StackChunk::is_parent_null<P>(as_oop()); }
inline void stackChunkOopDesc::set_parent(stackChunkOop value) { jdk_internal_vm_StackChunk::set_parent(this, value); }
template<typename P>
inline void stackChunkOopDesc::set_parent_raw(oop value)       { jdk_internal_vm_StackChunk::set_parent_raw<P>(this, value); }

inline int stackChunkOopDesc::stack_size() const        { return jdk_internal_vm_StackChunk::size(as_oop()); }

inline int stackChunkOopDesc::sp() const                { return jdk_internal_vm_StackChunk::sp(as_oop()); }
inline void stackChunkOopDesc::set_sp(int value)        { jdk_internal_vm_StackChunk::set_sp(this, value); }

inline address stackChunkOopDesc::pc() const            { return (address)jdk_internal_vm_StackChunk::pc(as_oop()); }
inline void stackChunkOopDesc::set_pc(address value)    { jdk_internal_vm_StackChunk::set_pc(this, (intptr_t)value); }

inline int stackChunkOopDesc::argsize() const           { return jdk_internal_vm_StackChunk::argsize(as_oop()); }
inline void stackChunkOopDesc::set_argsize(int value)   { jdk_internal_vm_StackChunk::set_argsize(as_oop(), value); }

inline uint8_t stackChunkOopDesc::flags() const         { return jdk_internal_vm_StackChunk::flags(as_oop()); }
inline void stackChunkOopDesc::set_flags(uint8_t value) { jdk_internal_vm_StackChunk::set_flags(this, value); }

inline int stackChunkOopDesc::max_size() const          { return jdk_internal_vm_StackChunk::maxSize(as_oop()); }
inline void stackChunkOopDesc::set_max_size(int value)  {
  assert(value >= 0, "size must be >= 0");
  jdk_internal_vm_StackChunk::set_maxSize(this, (jint)value);
}

inline oop stackChunkOopDesc::cont() const              { return UseCompressedOops ? cont<narrowOop>() : cont<oop>(); /* jdk_internal_vm_StackChunk::cont(as_oop()); */ }
template<typename P>
inline oop stackChunkOopDesc::cont() const              {
  oop obj = jdk_internal_vm_StackChunk::cont_raw<P>(as_oop());
  obj = (oop)NativeAccess<>::oop_load(&obj);
  return obj;
}
inline void stackChunkOopDesc::set_cont(oop value)      { jdk_internal_vm_StackChunk::set_cont(this, value); }
template<typename P>
inline void stackChunkOopDesc::set_cont_raw(oop value)  {  jdk_internal_vm_StackChunk::set_cont_raw<P>(this, value); }

inline int stackChunkOopDesc::bottom() const { return stack_size() - argsize(); }

inline HeapWord* stackChunkOopDesc::start_of_stack() const {
   return (HeapWord*)(cast_from_oop<intptr_t>(as_oop()) + InstanceStackChunkKlass::offset_of_stack());
}

inline intptr_t* stackChunkOopDesc::start_address() const { return (intptr_t*)start_of_stack(); }
inline intptr_t* stackChunkOopDesc::end_address() const { return start_address() + stack_size(); }
inline intptr_t* stackChunkOopDesc::bottom_address() const { return start_address() + bottom(); }
inline intptr_t* stackChunkOopDesc::sp_address()  const { return start_address() + sp(); }

inline int stackChunkOopDesc::to_offset(intptr_t* p) const {
  assert(is_in_chunk(p)
    || (p >= start_address() && (p - start_address()) <= stack_size() + frame::metadata_words),
    "p: " INTPTR_FORMAT " start: " INTPTR_FORMAT " end: " INTPTR_FORMAT, p2i(p), p2i(start_address()), p2i(bottom_address()));
  return p - start_address();
}

inline intptr_t* stackChunkOopDesc::from_offset(int offset) const {
  assert(offset <= stack_size(), "");
  return start_address() + offset;
}

inline bool stackChunkOopDesc::is_empty() const {
  return sp() >= stack_size() - argsize();
}

inline bool stackChunkOopDesc::is_in_chunk(void* p) const {
  HeapWord* start = (HeapWord*)start_address();
  HeapWord* end = start + stack_size();
  return (HeapWord*)p >= start && (HeapWord*)p < end;
}

bool stackChunkOopDesc::is_usable_in_chunk(void* p) const {
#if (defined(X86) || defined(AARCH64)) && !defined(ZERO)
  HeapWord* start = (HeapWord*)start_address() + sp() - frame::sender_sp_offset;
#else
  Unimplemented();
  HeapWord* start = NULL;
#endif
  HeapWord* end = start + stack_size();
  return (HeapWord*)p >= start && (HeapWord*)p < end;
}

inline bool stackChunkOopDesc::is_flag(uint8_t flag) const {
  return (flags() & flag) != 0;
}
inline void stackChunkOopDesc::set_flag(uint8_t flag, bool value) {
  uint32_t flags = this->flags();
  set_flags((uint8_t)(value ? flags |= flag : flags &= ~flag));
}
inline void stackChunkOopDesc::clear_flags() {
  set_flags(0);
}

inline bool stackChunkOopDesc::has_mixed_frames() const            { return is_flag(FLAG_HAS_INTERPRETED_FRAMES); }
inline void stackChunkOopDesc::set_has_mixed_frames(bool value)    { set_flag(FLAG_HAS_INTERPRETED_FRAMES, value); }

inline bool stackChunkOopDesc::is_gc_mode() const                  { return is_flag(FLAG_GC_MODE); }
inline void stackChunkOopDesc::set_gc_mode(bool value)             { set_flag(FLAG_GC_MODE, value); }

inline bool stackChunkOopDesc::has_bitmap() const                  { return is_flag(FLAG_HAS_BITMAP); }
inline void stackChunkOopDesc::set_has_bitmap(bool value)          { set_flag(FLAG_HAS_BITMAP, value); }

inline bool stackChunkOopDesc::has_thaw_slowpath_condition() const { return flags() != 0; }

inline bool stackChunkOopDesc::requires_barriers() {
  return Universe::heap()->requires_barriers(this);
}

template <stackChunkOopDesc::BarrierType barrier, ChunkFrames frame_kind, typename RegisterMapT>
void stackChunkOopDesc::do_barriers(const StackChunkFrameStream<frame_kind>& f, const RegisterMapT* map) {
  if (frame_kind == ChunkFrames::Mixed) {
    // we could freeze deopted frames in slow mode.
    f.handle_deopted();
  }
  do_barriers0<barrier>(f, map);
}

template <class StackChunkFrameClosureType>
inline void stackChunkOopDesc::iterate_stack(StackChunkFrameClosureType* closure) {
  has_mixed_frames() ? iterate_stack<ChunkFrames::Mixed>(closure)
                     : iterate_stack<ChunkFrames::CompiledOnly>(closure);
}

template <ChunkFrames frame_kind, class StackChunkFrameClosureType>
inline void stackChunkOopDesc::iterate_stack(StackChunkFrameClosureType* closure) {
  const SmallRegisterMap* map = SmallRegisterMap::instance;
  assert(!map->in_cont(), "");

  StackChunkFrameStream<frame_kind> f(this);
  bool should_continue = true;

  if (f.is_stub()) {
    RegisterMap full_map((JavaThread*)nullptr, true, false, true);
    full_map.set_include_argument_oops(false);

    f.next(&full_map);

    assert(!f.is_done(), "");
    assert(f.is_compiled(), "");

    should_continue = closure->do_frame(f, &full_map);
    f.next(map);
    f.handle_deopted(); // the stub caller might be deoptimized (as it's not at a call)
  }
  assert(!f.is_stub(), "");

  for(; should_continue && !f.is_done(); f.next(map)) {
    if (frame_kind == ChunkFrames::Mixed) {
      // in slow mode we might freeze deoptimized frames
      f.handle_deopted();
    }
    should_continue = closure->do_frame(f, map);
  }
}

inline frame stackChunkOopDesc::relativize(frame fr)   const { relativize_frame(fr);   return fr; }
inline frame stackChunkOopDesc::derelativize(frame fr) const { derelativize_frame(fr); return fr; }

inline BitMapView stackChunkOopDesc::bitmap() const {
  int stack_sz = stack_size();

  // The bitmap is located after the stack.
  HeapWord* bitmap_addr = start_of_stack() + stack_sz;
  size_t bitmap_size_in_bits = InstanceStackChunkKlass::bitmap_size_in_bits(stack_sz);

  BitMapView bitmap((BitMap::bm_word_t*)bitmap_addr, bitmap_size_in_bits);

  DEBUG_ONLY(bitmap.verify_range(bit_index_for(start_address()), bit_index_for(end_address()));)

  return bitmap;
}

inline BitMap::idx_t stackChunkOopDesc::bit_index_for(intptr_t* p) const {
  return UseCompressedOops ? bit_index_for((narrowOop*)p) : bit_index_for((oop*)p);
}

template <typename OopT>
inline BitMap::idx_t stackChunkOopDesc::bit_index_for(OopT* p) const {
  assert(p >= (OopT*)start_address(), "Address not in chunk");
  return p - (OopT*)start_address();
}

inline intptr_t* stackChunkOopDesc::address_for_bit(BitMap::idx_t index) const {
  return UseCompressedOops ? (intptr_t*)address_for_bit<narrowOop>(index) : (intptr_t*)address_for_bit<oop>(index);
}

template <typename OopT>
inline OopT* stackChunkOopDesc::address_for_bit(BitMap::idx_t index) const {
  return (OopT*)start_address() + index;
}

inline MemRegion stackChunkOopDesc::range() {
  return MemRegion((HeapWord*)this, size());
}

inline int stackChunkOopDesc::relativize_usp_offset(const frame& fr, const int usp_offset_in_bytes) const {
  assert(fr.is_compiled_frame() || fr.cb()->is_safepoint_stub(), "");
  assert(is_in_chunk(fr.unextended_sp()), "");

  intptr_t* base = fr.real_fp(); // equal to the caller's sp
  intptr_t* loc = (intptr_t*)((address)fr.unextended_sp() + usp_offset_in_bytes);
  assert(base > loc, "");
  return (int)(base - loc);
}

inline address stackChunkOopDesc::usp_offset_to_location(const frame& fr, const int usp_offset_in_bytes) const {
  assert(fr.is_compiled_frame(), "");
  return (address)derelativize_address(fr.offset_unextended_sp()) + usp_offset_in_bytes;
}

inline address stackChunkOopDesc::reg_to_location(const frame& fr, const RegisterMap* map, VMReg reg) const {
  assert(fr.is_compiled_frame(), "");
  assert(map != nullptr, "");
  assert(map->stack_chunk() == as_oop(), "");

  // the offsets are saved in the map after going through relativize_usp_offset, so they are sp - loc, in words
  intptr_t offset = (intptr_t)map->location(reg, nullptr); // see usp_offset_to_index for the chunk case
  intptr_t* base = derelativize_address(fr.offset_sp());
  return (address)(base - offset);
}

inline Method* stackChunkOopDesc::interpreter_frame_method(const frame& fr) {
  return derelativize(fr).interpreter_frame_method();
}

inline address stackChunkOopDesc::interpreter_frame_bcp(const frame& fr) {
  return derelativize(fr).interpreter_frame_bcp();
}

inline intptr_t* stackChunkOopDesc::interpreter_frame_expression_stack_at(const frame& fr, int index) const {
  frame heap_frame = derelativize(fr);
  assert(heap_frame.is_heap_frame(), "must be");
  return heap_frame.interpreter_frame_expression_stack_at(index);
}

inline intptr_t* stackChunkOopDesc::interpreter_frame_local_at(const frame& fr, int index) const {
  frame heap_frame = derelativize(fr);
  assert(heap_frame.is_heap_frame(), "must be");
  return heap_frame.interpreter_frame_local_at(index);
}

inline void stackChunkOopDesc::copy_from_stack_to_chunk(intptr_t* from, intptr_t* to, int size) {
  log_develop_trace(continuations)("Copying from v: " INTPTR_FORMAT " - " INTPTR_FORMAT " (%d words, %d bytes)",
    p2i(from), p2i(from + size), size, size << LogBytesPerWord);
  log_develop_trace(continuations)("Copying to h: " INTPTR_FORMAT "(" INTPTR_FORMAT "," INTPTR_FORMAT ") - " INTPTR_FORMAT "(" INTPTR_FORMAT "," INTPTR_FORMAT ") (%d words, %d bytes)",
    p2i(to), to - start_address(), relative_base() - to, p2i(to + size), to + size - start_address(),
    relative_base() - (to + size), size, size << LogBytesPerWord);

  assert(to >= start_address(), "");
  assert(to + size <= end_address(), "");

  memcpy(to, from, size << LogBytesPerWord);
}

inline void stackChunkOopDesc::copy_from_chunk_to_stack(intptr_t* from, intptr_t* to, int size) {
  log_develop_trace(continuations)("Copying from h: " INTPTR_FORMAT "(" INTPTR_FORMAT "," INTPTR_FORMAT ") - " INTPTR_FORMAT "(" INTPTR_FORMAT "," INTPTR_FORMAT ") (%d words, %d bytes)",
    p2i(from), from - start_address(), relative_base() - from, p2i(from + size), from + size - start_address(),
    relative_base() - (from + size), size, size << LogBytesPerWord);
  log_develop_trace(continuations)("Copying to v: " INTPTR_FORMAT " - " INTPTR_FORMAT " (%d words, %d bytes)", p2i(to),
    p2i(to + size), size, size << LogBytesPerWord);

  assert(from >= start_address(), "");
  assert(from + size <= end_address(), "");

#if !defined(AMD64) || !defined(AARCH64) || defined(ZERO)
  // Suppress compilation error from dummy function (somewhere).
  if (to != nullptr)
#endif
  memcpy(to, from, size << LogBytesPerWord);
}

inline intptr_t* stackChunkOopDesc::relative_base() const {
  // we relativize with respect to end rather than start because GC might compact the chunk
  return end_address() + frame::metadata_words;
}

inline intptr_t* stackChunkOopDesc::derelativize_address(int offset) const {
  intptr_t* base = relative_base();
  intptr_t* p = base - offset;
  assert(start_address() <= p && p <= base, "start_address: " PTR_FORMAT " p: " PTR_FORMAT " base: " PTR_FORMAT,
         p2i(start_address()), p2i(p), p2i(base));
  return p;
}

inline int stackChunkOopDesc::relativize_address(intptr_t* p) const {
  intptr_t* base = relative_base();
  intptr_t offset = base - p;
  assert(start_address() <= p && p <= base, "start_address: " PTR_FORMAT " p: " PTR_FORMAT " base: " PTR_FORMAT,
         p2i(start_address()), p2i(p), p2i(base));
  assert(0 <= offset && offset <= std::numeric_limits<int>::max(), "offset: " PTR_FORMAT, offset);
  return offset;
}

inline void stackChunkOopDesc::relativize_frame(frame& fr) const {
  fr.set_offset_sp(relativize_address(fr.sp()));
  fr.set_offset_unextended_sp(relativize_address(fr.unextended_sp()));
  relativize_frame_pd(fr);
}

inline void stackChunkOopDesc::derelativize_frame(frame& fr) const {
  fr.set_sp(derelativize_address(fr.offset_sp()));
  fr.set_unextended_sp(derelativize_address(fr.offset_unextended_sp()));
  derelativize_frame_pd(fr);
  fr.set_frame_index(-1); // for the sake of assertions in frame
}

#endif // SHARE_OOPS_STACKCHUNKOOP_INLINE_HPP
