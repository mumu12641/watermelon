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
  %n1 = alloca i32, align 4 
  store i32 %n, i32* %n1, align 4 
  %0 = load i32, i32* %n1, align 4 
  %1 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([3 x i8], [3 x i8]* @print_int_format, i32 0, i32 0), i32 %0) 
  ret void 
}

define void @print_string(i8* %str) { 
entry: 
  %str1 = alloca i8*, align 8 
  store i8* %str, i8** %str1, align 8 
  %0 = load i8*, i8** %str1, align 8 
  %1 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([3 x i8], [3 x i8]* @print_string_format, i32 0, i32 0), i8* %0) 
  ret void 
}

define void @print_double(double %n) { 
entry: 
  %n1 = alloca double, align 8 
  store double %n, double* %n1, align 8 
  %0 = load double, double* %n1, align 8 
  %1 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([3 x i8], [3 x i8]* @print_double_format, i32 0, i32 0), double %0) 
  ret void 
}

define void @print_bool(i1 %b) { 
entry: 
  %b1 = alloca i1, align 1 
  store i1 %b, i1* %b1, align 1 
  %0 = load i1, i1* %b1, align 1 
  br i1 %0, label %true_branch, label %false_branch true_branch: 
  %1 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([5 x i8], [5 x i8]* @print_bool_true, i32 0, i32 0)) 
  br label %end false_branch: 
  %2 = call i32(i8*, ...) @printf(i8* getelementptr inbounds([6 x i8], [6 x i8]* @print_bool_false, i32 0, i32 0)) 
  br label %end end: 
  ret void 
}