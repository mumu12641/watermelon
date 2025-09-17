; ModuleID = 'tests/test_pass_cpp.ll'

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

@int_format = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@float_format = private unnamed_addr constant [3 x i8] c"%f\00", align 1
@bool_true_str = private unnamed_addr constant [5 x i8] c"true\00", align 1
@bool_false_str = private unnamed_addr constant [6 x i8] c"false\00", align 1
@empty_str = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

declare i8* @gc_alloc(i64)

declare i64 @strlen(i8*)

declare i8* @strcpy(i8*, i8*)

declare i8* @strcat(i8*, i8*)

declare i32 @sprintf(i8*, i8*, ...)

declare i32 @strcmp(i8*, i8*)

declare double @atof(i8*)

declare i32 @atoi(i8*)

define i8* @_concat_strs(i8* %str1, i8* %str2) {
entry:
  %str1_null = icmp eq i8* %str1, null
  %str2_null = icmp eq i8* %str2, null
  br i1 %str1_null, label %handle_str1_null, label %check_str2

handle_str1_null:                                 ; preds = %entry
  br i1 %str2_null, label %both_null, label %str1_null_str2_valid

check_str2:                                       ; preds = %entry
  br i1 %str2_null, label %str2_null_str1_valid, label %both_valid

both_null:                                        ; preds = %handle_str1_null
  %empty_str = call noalias i8* @gc_alloc(i64 1)
  store i8 0, i8* %empty_str, align 1
  ret i8* %empty_str

str1_null_str2_valid:                             ; preds = %handle_str1_null
  %len2_only = call i64 @strlen(i8* %str2)
  %size2_only = add i64 %len2_only, 1
  %result2_only = call noalias i8* @gc_alloc(i64 %size2_only)
  %copied2_only = call i8* @strcpy(i8* %result2_only, i8* %str2)
  ret i8* %result2_only

str2_null_str1_valid:                             ; preds = %check_str2
  %len1_only = call i64 @strlen(i8* %str1)
  %size1_only = add i64 %len1_only, 1
  %result1_only = call noalias i8* @gc_alloc(i64 %size1_only)
  %copied1_only = call i8* @strcpy(i8* %result1_only, i8* %str1)
  ret i8* %result1_only

both_valid:                                       ; preds = %check_str2
  %len1 = call i64 @strlen(i8* %str1)
  %len2 = call i64 @strlen(i8* %str2)
  %total_len = add i64 %len1, %len2
  %total_size = add i64 %total_len, 1
  %result = call noalias i8* @gc_alloc(i64 %total_size)
  %malloc_success = icmp ne i8* %result, null
  br i1 %malloc_success, label %copy_strings, label %malloc_failed

malloc_failed:                                    ; preds = %both_valid
  ret i8* null

copy_strings:                                     ; preds = %both_valid
  %copied1 = call i8* @strcpy(i8* %result, i8* %str1)
  %final_result = call i8* @strcat(i8* %result, i8* %str2)
  ret i8* %result
}

define i8* @int_to_str(i32 %value) {
entry:
  %buffer = call noalias i8* @gc_alloc(i64 32)
  %alloc_success = icmp ne i8* %buffer, null
  br i1 %alloc_success, label %convert, label %alloc_failed

alloc_failed:                                     ; preds = %entry
  ret i8* null

convert:                                          ; preds = %entry
  %format_ptr = getelementptr inbounds [3 x i8], [3 x i8]* @int_format, i64 0, i64 0
  %sprintf_result = call i32 (i8*, i8*, ...) @sprintf(i8* %buffer, i8* %format_ptr, i32 %value)
  %convert_success = icmp sgt i32 %sprintf_result, 0
  br i1 %convert_success, label %success, label %convert_failed

convert_failed:                                   ; preds = %convert
  ret i8* null

success:                                          ; preds = %convert
  ret i8* %buffer
}

define i8* @float_to_str(float %value) {
entry:
  %buffer = call noalias i8* @gc_alloc(i64 64)
  %alloc_success = icmp ne i8* %buffer, null
  br i1 %alloc_success, label %convert, label %alloc_failed

alloc_failed:                                     ; preds = %entry
  ret i8* null

convert:                                          ; preds = %entry
  %value_double = fpext float %value to double
  %format_ptr = getelementptr inbounds [3 x i8], [3 x i8]* @float_format, i64 0, i64 0
  %sprintf_result = call i32 (i8*, i8*, ...) @sprintf(i8* %buffer, i8* %format_ptr, double %value_double)
  %convert_success = icmp sgt i32 %sprintf_result, 0
  br i1 %convert_success, label %success, label %convert_failed

convert_failed:                                   ; preds = %convert
  ret i8* null

success:                                          ; preds = %convert
  ret i8* %buffer
}

define i8* @bool_to_str(i1 %value) {
entry:
  br i1 %value, label %true_branch, label %false_branch

true_branch:                                      ; preds = %entry
  %true_buffer = call noalias i8* @gc_alloc(i64 5)
  %true_alloc_success = icmp ne i8* %true_buffer, null
  br i1 %true_alloc_success, label %copy_true, label %true_alloc_failed

true_alloc_failed:                                ; preds = %true_branch
  ret i8* null

copy_true:                                        ; preds = %true_branch
  %true_str_ptr = getelementptr inbounds [5 x i8], [5 x i8]* @bool_true_str, i64 0, i64 0
  %true_result = call i8* @strcpy(i8* %true_buffer, i8* %true_str_ptr)
  ret i8* %true_buffer

false_branch:                                     ; preds = %entry
  %false_buffer = call noalias i8* @gc_alloc(i64 6)
  %false_alloc_success = icmp ne i8* %false_buffer, null
  br i1 %false_alloc_success, label %copy_false, label %false_alloc_failed

false_alloc_failed:                               ; preds = %false_branch
  ret i8* null

copy_false:                                       ; preds = %false_branch
  %false_str_ptr = getelementptr inbounds [6 x i8], [6 x i8]* @bool_false_str, i64 0, i64 0
  %false_result = call i8* @strcpy(i8* %false_buffer, i8* %false_str_ptr)
  ret i8* %false_buffer
}

define i32 @str_to_int(i8* %str) {
entry:
  %str_null = icmp eq i8* %str, null
  br i1 %str_null, label %invalid_input, label %check_empty

check_empty:                                      ; preds = %entry
  %len = call i64 @strlen(i8* %str)
  %is_empty = icmp eq i64 %len, 0
  br i1 %is_empty, label %invalid_input, label %convert

convert:                                          ; preds = %check_empty
  %result = call i32 @atoi(i8* %str)
  ret i32 %result

invalid_input:                                    ; preds = %check_empty, %entry
  ret i32 0
}

define float @str_to_float(i8* %str) {
entry:
  %str_null = icmp eq i8* %str, null
  br i1 %str_null, label %invalid_input, label %check_empty

check_empty:                                      ; preds = %entry
  %len = call i64 @strlen(i8* %str)
  %is_empty = icmp eq i64 %len, 0
  br i1 %is_empty, label %invalid_input, label %convert

convert:                                          ; preds = %check_empty
  %result_double = call double @atof(i8* %str)
  %result_float = fptrunc double %result_double to float
  ret float %result_float

invalid_input:                                    ; preds = %check_empty, %entry
  ret float 0.000000e+00
}

define i1 @str_to_bool(i8* %str) {
entry:
  %str_null = icmp eq i8* %str, null
  br i1 %str_null, label %return_false, label %check_empty

check_empty:                                      ; preds = %entry
  %len = call i64 @strlen(i8* %str)
  %is_empty = icmp eq i64 %len, 0
  br i1 %is_empty, label %return_false, label %check_true

check_true:                                       ; preds = %check_empty
  %true_str_ptr = getelementptr inbounds [5 x i8], [5 x i8]* @bool_true_str, i64 0, i64 0
  %cmp_true = call i32 @strcmp(i8* %str, i8* %true_str_ptr)
  %is_true = icmp eq i32 %cmp_true, 0
  br i1 %is_true, label %return_true, label %check_false

check_false:                                      ; preds = %check_true
  %false_str_ptr = getelementptr inbounds [6 x i8], [6 x i8]* @bool_false_str, i64 0, i64 0
  %cmp_false = call i32 @strcmp(i8* %str, i8* %false_str_ptr)
  %is_false = icmp eq i32 %cmp_false, 0
  br i1 %is_false, label %return_false, label %return_false

return_true:                                      ; preds = %check_true
  ret i1 true

return_false:                                     ; preds = %check_false, %check_false, %check_empty, %entry
  ret i1 false
}
