@println_format = private unnamed_addr constant [2 x i8] c "\0A\00", align 1
@print_int_format = private unnamed_addr constant [3 x i8] c "%d\00", align 1
@print_string_format = private unnamed_addr constant [3 x i8] c "%s\00", align 1
@print_double_format = private unnamed_addr constant [3 x i8] c "%f\00", align 1
@print_bool_true = private unnamed_addr constant [5 x i8] c "true\00", align 1
@print_bool_false = private unnamed_addr constant [6 x i8] c "false\00", align 1

declare i32 @printf(i8*, ...)

define void @println() { 
entry: 
  %0 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([2 x i8], [2 x i8]* @println_format, i32 0, i32 0)) 
  ret void 
}

define void @print_int(i32 %n) { 
entry: 
  %0 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([3 x i8], [3 x i8]* @print_int_format, i32 0, i32 0), i32 %n) 
  ret void 
}

define void @print_str(i8* %str) { 
entry: 
  %0 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([3 x i8], [3 x i8]* @print_string_format, i32 0, i32 0), i8* %str) 
  ret void 
}

define void @print_double(double %n) { 
entry: 
  %0 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([3 x i8], [3 x i8]* @print_double_format, i32 0, i32 0), double %n) 
  ret void 
}

define void @print_bool(i1 %b) { 
entry: 
  br i1 %b, label %true_branch, label %false_branch true_branch: 
  ; preds = %entry
  %0 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([5 x i8], [5 x i8]* @print_bool_true, i32 0, i32 0)) 
  br label %end false_branch: 
  ; preds = %entry
  %1 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([6 x i8], [6 x i8]* @print_bool_false, i32 0, i32 0)) 
  br label %end end: 
  ; preds = %false_branch, %true_branch
  ret void 
}

%struct._IO_FILE = type opaque
@stdin = external global %struct._IO_FILE*

declare i8* @fgets(i8*, i32, %struct._IO_FILE*)

declare i8* @gc_alloc(i64)

declare i64 @strlen(i8*)

define i8* @_input_str_safe(i32 %max_length) { 
entry: 
  %buffer_size = add i32 %max_length, 1 
  %buffer_size_i64 = zext i32 %buffer_size to i64 
  %buffer = call noalias i8* @gc_alloc(i64 %buffer_size_i64) 
  %alloc_success = icmp ne i8* %buffer, null 
  br i1 %alloc_success, label %read_input, label %alloc_failed alloc_failed: 
  ; preds = %entry
  ret i8* null read_input: 
  ; preds = %entry
  %stdin_ptr = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 8 
  %fgets_result = call i8* @fgets(i8* %buffer, i32 %buffer_size, %struct._IO_FILE* %stdin_ptr) 
  %read_success = icmp ne i8* %fgets_result, null 
  br i1 %read_success, label %process_input, label %read_failed read_failed: 
  ; preds = %read_input
  ret i8* null process_input: 
  ; preds = %read_input
  %len = call i64 @strlen(i8* %buffer) 
  %len_i32 = trunc i64 %len to i32 
  %has_content = icmp sgt i32 %len_i32, 0 
  br i1 %has_content, label %check_newline, label %success check_newline: 
  ; preds = %process_input
  %last_idx = sub i32 %len_i32, 1 
  %last_idx_i64 = zext i32 %last_idx to i64 
  %last_char_ptr = getelementptr inbounds i8, i8* %buffer, i64 %last_idx_i64 
  %last_char = load i8, i8* %last_char_ptr, align 1 
  %is_newline = icmp eq i8 %last_char, 10 
  br i1 %is_newline, label %remove_newline, label %success remove_newline: 
  ; preds = %check_newline
  store i8 0, i8* %last_char_ptr, align 1 
  br label %success success: 
  ; preds = %remove_newline, %check_newline, %process_input
  ret i8* %buffer 
}

define i8* @input() { 
entry: 
  %result = call i8* @_input_str_safe(i32 1023) 
  ret i8* %result 
}
