; Copyright (c) 2025 muuuuuu_02

; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:

; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.

; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.

; for convenience
%vTable_IntArray.local = type { void(i8*)*, %IntArray.local*(i8*, i32)*, %IntArray.local*(i32)*, void(i8*)*, i32(i8*, i32)*, i32(i8*)*, i1(i8*)*, void(i8*, i32)*, i32(i8*)*, void(i8*, i32)*, i32(i8*)* }
%IntArray.local = type { %vTable_IntArray.local*, i32, i32, i32, i8*, i32 }

declare i8* @gc_alloc(i64)

define i8* @_builtin_malloc(i32 %size) { 
entry: 
  %size_i64 = sext i32 %size to i64 
  %mem = call noalias i8* @gc_alloc(i64 %size_i64) 
  ret i8* %mem 
}

define void @_builtin_int_array_insert_impl(i8* %0, i32 %element) { 
entry: 
  %self1 = bitcast i8* %0 to %IntArray.local* 
  %_data_ptr = getelementptr inbounds %IntArray.local, %IntArray.local* %self1, i32 0, i32 4 
  %_data = load i8*, i8** %_data_ptr, align 4 
  %_size_ptr = getelementptr inbounds %IntArray.local, %IntArray.local* %self1, i32 0, i32 5 
  %_size = load i32, i32* %_size_ptr, align 4 
  %byte_offset = mul i32 %_size, 4 
  %insert_ptr = getelementptr inbounds i8, i8* %_data, i32 %byte_offset 
  %insert_ptr_typed = bitcast i8* %insert_ptr to i32* 
  store i32 %element, i32* %insert_ptr_typed, align 4 
  ret void 
}

define i32 @_builtin_int_array_at_impl(i8* %0, i32 %index) { 
entry: 
  %self1 = bitcast i8* %0 to %IntArray.local* 
  %_data_ptr = getelementptr inbounds %IntArray.local, %IntArray.local* %self1, i32 0, i32 4 
  %_data = load i8*, i8** %_data_ptr, align 4 
  %byte_offset = mul i32 %index, 4 
  %at_ptr = getelementptr inbounds i8, i8* %_data, i32 %byte_offset 
  %at_ptr_typed = bitcast i8* %at_ptr to i32* 
  %at = load i32, i32* %at_ptr_typed, align 4 
  ret i32 %at 
}