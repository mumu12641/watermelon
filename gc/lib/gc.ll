; ModuleID = 'src/gc.cpp'
source_filename = "src/gc.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.GC = type { %struct.GCPtr*, %struct.GCPtr*, i64, i64, i64, i64, i64, i64, i8, i8*, double }
%struct.GCPtr = type <{ i8*, i64, i64, i32, [4 x i8] }>
%struct.__jmp_buf_tag = type { [8 x i64], i32, %struct.__sigset_t }
%struct.__sigset_t = type { [16 x i64] }
%struct.A = type { %struct.B* }
%struct.B = type { %struct.A* }

$_ZN5GCPtrC2EPvmm = comdat any

$_ZN5GCPtrC2Ev = comdat any

$_ZNK5GCPtr7isEmptyEv = comdat any

$_ZNK5GCPtr10isOccupiedEv = comdat any

$_ZN5GCPtr5clearEv = comdat any

$_ZN2GCC2Ev = comdat any

@_ZL6PRIMES = internal constant [23 x i64] [i64 1, i64 5, i64 11, i64 23, i64 53, i64 101, i64 197, i64 389, i64 683, i64 1259, i64 2417, i64 4733, i64 9371, i64 18617, i64 37097, i64 74093, i64 148073, i64 296099, i64 592019, i64 1100009, i64 2200013, i64 4400021, i64 8800019], align 16
@_ZL2gc = internal global %struct.GC zeroinitializer, align 8
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_gc.cpp, i8* null }]

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i64 @_ZN2GC4hashEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1) #0 align 2 {
  %3 = alloca %struct.GC*, align 8
  %4 = alloca i8*, align 8
  %5 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %3, align 8
  store i8* %1, i8** %4, align 8
  %6 = load %struct.GC*, %struct.GC** %3, align 8
  %7 = load i8*, i8** %4, align 8
  %8 = ptrtoint i8* %7 to i64
  store i64 %8, i64* %5, align 8
  %9 = load i64, i64* %5, align 8
  %10 = mul i64 13, %9
  %11 = load i64, i64* %5, align 8
  %12 = lshr i64 %11, 15
  %13 = xor i64 %10, %12
  ret i64 %13
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i64 @_ZN2GC12getPrimeSizeEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i64 noundef %1) #0 align 2 {
  %3 = alloca i64, align 8
  %4 = alloca %struct.GC*, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %4, align 8
  store i64 %1, i64* %5, align 8
  %7 = load %struct.GC*, %struct.GC** %4, align 8
  store i64 0, i64* %6, align 8
  br label %8

8:                                                ; preds = %22, %2
  %9 = load i64, i64* %6, align 8
  %10 = icmp ult i64 %9, 23
  br i1 %10, label %11, label %25

11:                                               ; preds = %8
  %12 = load i64, i64* %6, align 8
  %13 = getelementptr inbounds [23 x i64], [23 x i64]* @_ZL6PRIMES, i64 0, i64 %12
  %14 = load i64, i64* %13, align 8
  %15 = load i64, i64* %5, align 8
  %16 = icmp ugt i64 %14, %15
  br i1 %16, label %17, label %21

17:                                               ; preds = %11
  %18 = load i64, i64* %6, align 8
  %19 = getelementptr inbounds [23 x i64], [23 x i64]* @_ZL6PRIMES, i64 0, i64 %18
  %20 = load i64, i64* %19, align 8
  store i64 %20, i64* %3, align 8
  br label %28

21:                                               ; preds = %11
  br label %22

22:                                               ; preds = %21
  %23 = load i64, i64* %6, align 8
  %24 = add i64 %23, 1
  store i64 %24, i64* %6, align 8
  br label %8, !llvm.loop !6

25:                                               ; preds = %8
  %26 = load i64, i64* %5, align 8
  %27 = mul i64 %26, 2
  store i64 %27, i64* %3, align 8
  br label %28

28:                                               ; preds = %25, %17
  %29 = load i64, i64* %3, align 8
  ret i64 %29
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i64 @_ZN2GC13probeDistanceEmm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i64 noundef %1, i64 noundef %2) #0 align 2 {
  %4 = alloca %struct.GC*, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %4, align 8
  store i64 %1, i64* %5, align 8
  store i64 %2, i64* %6, align 8
  %8 = load %struct.GC*, %struct.GC** %4, align 8
  %9 = load i64, i64* %5, align 8
  %10 = load i64, i64* %6, align 8
  %11 = sub i64 %10, 1
  %12 = sub i64 %9, %11
  store i64 %12, i64* %7, align 8
  %13 = load i64, i64* %7, align 8
  %14 = icmp slt i64 %13, 0
  br i1 %14, label %15, label %20

15:                                               ; preds = %3
  %16 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 2
  %17 = load i64, i64* %16, align 8
  %18 = load i64, i64* %7, align 8
  %19 = add i64 %17, %18
  br label %22

20:                                               ; preds = %3
  %21 = load i64, i64* %7, align 8
  br label %22

22:                                               ; preds = %20, %15
  %23 = phi i64 [ %19, %15 ], [ %21, %20 ]
  ret i64 %23
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef %struct.GCPtr* @_ZN2GC13searchPtrImplEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1) #0 align 2 {
  %3 = alloca %struct.GCPtr*, align 8
  %4 = alloca %struct.GC*, align 8
  %5 = alloca i8*, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca %struct.GCPtr, align 8
  store %struct.GC* %0, %struct.GC** %4, align 8
  store i8* %1, i8** %5, align 8
  %9 = load %struct.GC*, %struct.GC** %4, align 8
  %10 = load i8*, i8** %5, align 8
  %11 = call noundef i64 @_ZN2GC4hashEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %9, i8* noundef %10)
  %12 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 2
  %13 = load i64, i64* %12, align 8
  %14 = urem i64 %11, %13
  store i64 %14, i64* %6, align 8
  store i64 0, i64* %7, align 8
  br label %15

15:                                               ; preds = %2, %43
  %16 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 0
  %17 = load %struct.GCPtr*, %struct.GCPtr** %16, align 8
  %18 = load i64, i64* %6, align 8
  %19 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %17, i64 %18
  %20 = bitcast %struct.GCPtr* %8 to i8*
  %21 = bitcast %struct.GCPtr* %19 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %20, i8* align 8 %21, i64 32, i1 false)
  %22 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %8, i32 0, i32 2
  %23 = load i64, i64* %22, align 8
  %24 = icmp eq i64 %23, 0
  br i1 %24, label %32, label %25

25:                                               ; preds = %15
  %26 = load i64, i64* %7, align 8
  %27 = load i64, i64* %6, align 8
  %28 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %8, i32 0, i32 2
  %29 = load i64, i64* %28, align 8
  %30 = call noundef i64 @_ZN2GC13probeDistanceEmm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %9, i64 noundef %27, i64 noundef %29)
  %31 = icmp ugt i64 %26, %30
  br i1 %31, label %32, label %33

32:                                               ; preds = %25, %15
  store %struct.GCPtr* null, %struct.GCPtr** %3, align 8
  br label %51

33:                                               ; preds = %25
  %34 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %8, i32 0, i32 0
  %35 = load i8*, i8** %34, align 8
  %36 = load i8*, i8** %5, align 8
  %37 = icmp eq i8* %35, %36
  br i1 %37, label %38, label %43

38:                                               ; preds = %33
  %39 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 0
  %40 = load %struct.GCPtr*, %struct.GCPtr** %39, align 8
  %41 = load i64, i64* %6, align 8
  %42 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %40, i64 %41
  store %struct.GCPtr* %42, %struct.GCPtr** %3, align 8
  br label %51

43:                                               ; preds = %33
  %44 = load i64, i64* %6, align 8
  %45 = add i64 %44, 1
  %46 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 2
  %47 = load i64, i64* %46, align 8
  %48 = urem i64 %45, %47
  store i64 %48, i64* %6, align 8
  %49 = load i64, i64* %7, align 8
  %50 = add i64 %49, 1
  store i64 %50, i64* %7, align 8
  br label %15, !llvm.loop !8

51:                                               ; preds = %38, %32
  %52 = load %struct.GCPtr*, %struct.GCPtr** %3, align 8
  ret %struct.GCPtr* %52
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC10addPtrImplEPvm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1, i64 noundef %2) #2 align 2 {
  %4 = alloca %struct.GC*, align 8
  %5 = alloca i8*, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca %struct.GCPtr, align 8
  %10 = alloca %struct.GCPtr, align 8
  %11 = alloca %struct.GCPtr, align 8
  %12 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %4, align 8
  store i8* %1, i8** %5, align 8
  store i64 %2, i64* %6, align 8
  %13 = load %struct.GC*, %struct.GC** %4, align 8
  %14 = load i8*, i8** %5, align 8
  %15 = call noundef i64 @_ZN2GC4hashEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %13, i8* noundef %14)
  %16 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 2
  %17 = load i64, i64* %16, align 8
  %18 = urem i64 %15, %17
  store i64 %18, i64* %7, align 8
  store i64 0, i64* %8, align 8
  %19 = load i8*, i8** %5, align 8
  %20 = load i64, i64* %6, align 8
  %21 = load i64, i64* %7, align 8
  %22 = add i64 %21, 1
  call void @_ZN5GCPtrC2EPvmm(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %9, i8* noundef %19, i64 noundef %20, i64 noundef %22)
  call void @_ZN5GCPtrC2Ev(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %10) #9
  br label %23

23:                                               ; preds = %3, %63
  %24 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %25 = load %struct.GCPtr*, %struct.GCPtr** %24, align 8
  %26 = load i64, i64* %7, align 8
  %27 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %25, i64 %26
  %28 = bitcast %struct.GCPtr* %11 to i8*
  %29 = bitcast %struct.GCPtr* %27 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %28, i8* align 8 %29, i64 32, i1 false)
  %30 = call noundef zeroext i1 @_ZNK5GCPtr7isEmptyEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %11)
  br i1 %30, label %31, label %41

31:                                               ; preds = %23
  %32 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %33 = load %struct.GCPtr*, %struct.GCPtr** %32, align 8
  %34 = load i64, i64* %7, align 8
  %35 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %33, i64 %34
  %36 = bitcast %struct.GCPtr* %35 to i8*
  %37 = bitcast %struct.GCPtr* %9 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %36, i8* align 8 %37, i64 28, i1 false)
  %38 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 3
  %39 = load i64, i64* %38, align 8
  %40 = add i64 %39, 1
  store i64 %40, i64* %38, align 8
  br label %71

41:                                               ; preds = %23
  %42 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %11, i32 0, i32 0
  %43 = load i8*, i8** %42, align 8
  %44 = load i8*, i8** %5, align 8
  %45 = icmp eq i8* %43, %44
  br i1 %45, label %46, label %47

46:                                               ; preds = %41
  br label %71

47:                                               ; preds = %41
  %48 = load i64, i64* %7, align 8
  %49 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %11, i32 0, i32 2
  %50 = load i64, i64* %49, align 8
  %51 = call noundef i64 @_ZN2GC13probeDistanceEmm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %13, i64 noundef %48, i64 noundef %50)
  store i64 %51, i64* %12, align 8
  %52 = load i64, i64* %8, align 8
  %53 = load i64, i64* %12, align 8
  %54 = icmp ugt i64 %52, %53
  br i1 %54, label %55, label %63

55:                                               ; preds = %47
  %56 = bitcast %struct.GCPtr* %10 to i8*
  %57 = bitcast %struct.GCPtr* %9 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %56, i8* align 8 %57, i64 28, i1 false)
  %58 = bitcast %struct.GCPtr* %9 to i8*
  %59 = bitcast %struct.GCPtr* %11 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %58, i8* align 8 %59, i64 28, i1 false)
  %60 = bitcast %struct.GCPtr* %11 to i8*
  %61 = bitcast %struct.GCPtr* %10 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %60, i8* align 8 %61, i64 28, i1 false)
  %62 = load i64, i64* %12, align 8
  store i64 %62, i64* %8, align 8
  br label %63

63:                                               ; preds = %55, %47
  %64 = load i64, i64* %7, align 8
  %65 = add i64 %64, 1
  %66 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 2
  %67 = load i64, i64* %66, align 8
  %68 = urem i64 %65, %67
  store i64 %68, i64* %7, align 8
  %69 = load i64, i64* %8, align 8
  %70 = add i64 %69, 1
  store i64 %70, i64* %8, align 8
  br label %23, !llvm.loop !9

71:                                               ; preds = %46, %31
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN5GCPtrC2EPvmm(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %0, i8* noundef %1, i64 noundef %2, i64 noundef %3) unnamed_addr #3 comdat align 2 {
  %5 = alloca %struct.GCPtr*, align 8
  %6 = alloca i8*, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  store %struct.GCPtr* %0, %struct.GCPtr** %5, align 8
  store i8* %1, i8** %6, align 8
  store i64 %2, i64* %7, align 8
  store i64 %3, i64* %8, align 8
  %9 = load %struct.GCPtr*, %struct.GCPtr** %5, align 8
  %10 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %9, i32 0, i32 0
  %11 = load i8*, i8** %6, align 8
  store i8* %11, i8** %10, align 8
  %12 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %9, i32 0, i32 1
  %13 = load i64, i64* %7, align 8
  store i64 %13, i64* %12, align 8
  %14 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %9, i32 0, i32 2
  %15 = load i64, i64* %8, align 8
  store i64 %15, i64* %14, align 8
  %16 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %9, i32 0, i32 3
  store i32 0, i32* %16, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN5GCPtrC2Ev(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca %struct.GCPtr*, align 8
  store %struct.GCPtr* %0, %struct.GCPtr** %2, align 8
  %3 = load %struct.GCPtr*, %struct.GCPtr** %2, align 8
  %4 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 0
  store i8* null, i8** %4, align 8
  %5 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 1
  store i64 0, i64* %5, align 8
  %6 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 2
  store i64 0, i64* %6, align 8
  %7 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 3
  store i32 0, i32* %7, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK5GCPtr7isEmptyEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %0) #0 comdat align 2 {
  %2 = alloca %struct.GCPtr*, align 8
  store %struct.GCPtr* %0, %struct.GCPtr** %2, align 8
  %3 = load %struct.GCPtr*, %struct.GCPtr** %2, align 8
  %4 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 2
  %5 = load i64, i64* %4, align 8
  %6 = icmp eq i64 %5, 0
  ret i1 %6
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC13removePtrImplEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1) #2 align 2 {
  %3 = alloca %struct.GC*, align 8
  %4 = alloca i8*, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca %struct.GCPtr*, align 8
  %9 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %3, align 8
  store i8* %1, i8** %4, align 8
  %10 = load %struct.GC*, %struct.GC** %3, align 8
  %11 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 3
  %12 = load i64, i64* %11, align 8
  %13 = icmp eq i64 %12, 0
  br i1 %13, label %14, label %15

14:                                               ; preds = %2
  br label %148

15:                                               ; preds = %2
  store i64 0, i64* %5, align 8
  br label %16

16:                                               ; preds = %37, %15
  %17 = load i64, i64* %5, align 8
  %18 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 4
  %19 = load i64, i64* %18, align 8
  %20 = icmp ult i64 %17, %19
  br i1 %20, label %21, label %40

21:                                               ; preds = %16
  %22 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 1
  %23 = load %struct.GCPtr*, %struct.GCPtr** %22, align 8
  %24 = load i64, i64* %5, align 8
  %25 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %23, i64 %24
  %26 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %25, i32 0, i32 0
  %27 = load i8*, i8** %26, align 8
  %28 = load i8*, i8** %4, align 8
  %29 = icmp eq i8* %27, %28
  br i1 %29, label %30, label %36

30:                                               ; preds = %21
  %31 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 1
  %32 = load %struct.GCPtr*, %struct.GCPtr** %31, align 8
  %33 = load i64, i64* %5, align 8
  %34 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %32, i64 %33
  %35 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %34, i32 0, i32 0
  store i8* null, i8** %35, align 8
  br label %36

36:                                               ; preds = %30, %21
  br label %37

37:                                               ; preds = %36
  %38 = load i64, i64* %5, align 8
  %39 = add i64 %38, 1
  store i64 %39, i64* %5, align 8
  br label %16, !llvm.loop !10

40:                                               ; preds = %16
  %41 = load i8*, i8** %4, align 8
  %42 = call noundef i64 @_ZN2GC4hashEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %10, i8* noundef %41)
  %43 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 2
  %44 = load i64, i64* %43, align 8
  %45 = urem i64 %42, %44
  store i64 %45, i64* %6, align 8
  store i64 0, i64* %7, align 8
  br label %46

46:                                               ; preds = %72, %40
  %47 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %48 = load %struct.GCPtr*, %struct.GCPtr** %47, align 8
  %49 = load i64, i64* %6, align 8
  %50 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %48, i64 %49
  %51 = call noundef zeroext i1 @_ZNK5GCPtr10isOccupiedEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %50)
  br i1 %51, label %52, label %80

52:                                               ; preds = %46
  %53 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %54 = load %struct.GCPtr*, %struct.GCPtr** %53, align 8
  %55 = load i64, i64* %6, align 8
  %56 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %54, i64 %55
  store %struct.GCPtr* %56, %struct.GCPtr** %8, align 8
  %57 = load %struct.GCPtr*, %struct.GCPtr** %8, align 8
  %58 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %57, i32 0, i32 0
  %59 = load i8*, i8** %58, align 8
  %60 = load i8*, i8** %4, align 8
  %61 = icmp eq i8* %59, %60
  br i1 %61, label %62, label %63

62:                                               ; preds = %52
  br label %80

63:                                               ; preds = %52
  %64 = load i64, i64* %7, align 8
  %65 = load i64, i64* %6, align 8
  %66 = load %struct.GCPtr*, %struct.GCPtr** %8, align 8
  %67 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %66, i32 0, i32 2
  %68 = load i64, i64* %67, align 8
  %69 = call noundef i64 @_ZN2GC13probeDistanceEmm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %10, i64 noundef %65, i64 noundef %68)
  %70 = icmp ugt i64 %64, %69
  br i1 %70, label %71, label %72

71:                                               ; preds = %63
  br label %148

72:                                               ; preds = %63
  %73 = load i64, i64* %6, align 8
  %74 = add i64 %73, 1
  %75 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 2
  %76 = load i64, i64* %75, align 8
  %77 = urem i64 %74, %76
  store i64 %77, i64* %6, align 8
  %78 = load i64, i64* %7, align 8
  %79 = add i64 %78, 1
  store i64 %79, i64* %7, align 8
  br label %46, !llvm.loop !11

80:                                               ; preds = %62, %46
  %81 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %82 = load %struct.GCPtr*, %struct.GCPtr** %81, align 8
  %83 = load i64, i64* %6, align 8
  %84 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %82, i64 %83
  %85 = call noundef zeroext i1 @_ZNK5GCPtr7isEmptyEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %84)
  br i1 %85, label %95, label %86

86:                                               ; preds = %80
  %87 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %88 = load %struct.GCPtr*, %struct.GCPtr** %87, align 8
  %89 = load i64, i64* %6, align 8
  %90 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %88, i64 %89
  %91 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %90, i32 0, i32 0
  %92 = load i8*, i8** %91, align 8
  %93 = load i8*, i8** %4, align 8
  %94 = icmp ne i8* %92, %93
  br i1 %94, label %95, label %96

95:                                               ; preds = %86, %80
  br label %148

96:                                               ; preds = %86
  %97 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %98 = load %struct.GCPtr*, %struct.GCPtr** %97, align 8
  %99 = load i64, i64* %6, align 8
  %100 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %98, i64 %99
  call void @_ZN5GCPtr5clearEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %100)
  %101 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 3
  %102 = load i64, i64* %101, align 8
  %103 = add i64 %102, -1
  store i64 %103, i64* %101, align 8
  %104 = load i64, i64* %6, align 8
  %105 = add i64 %104, 1
  %106 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 2
  %107 = load i64, i64* %106, align 8
  %108 = urem i64 %105, %107
  store i64 %108, i64* %9, align 8
  br label %109

109:                                              ; preds = %127, %96
  %110 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %111 = load %struct.GCPtr*, %struct.GCPtr** %110, align 8
  %112 = load i64, i64* %9, align 8
  %113 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %111, i64 %112
  %114 = call noundef zeroext i1 @_ZNK5GCPtr10isOccupiedEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %113)
  br i1 %114, label %115, label %125

115:                                              ; preds = %109
  %116 = load i64, i64* %9, align 8
  %117 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %118 = load %struct.GCPtr*, %struct.GCPtr** %117, align 8
  %119 = load i64, i64* %9, align 8
  %120 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %118, i64 %119
  %121 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %120, i32 0, i32 2
  %122 = load i64, i64* %121, align 8
  %123 = call noundef i64 @_ZN2GC13probeDistanceEmm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %10, i64 noundef %116, i64 noundef %122)
  %124 = icmp ugt i64 %123, 0
  br label %125

125:                                              ; preds = %115, %109
  %126 = phi i1 [ false, %109 ], [ %124, %115 ]
  br i1 %126, label %127, label %148

127:                                              ; preds = %125
  %128 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %129 = load %struct.GCPtr*, %struct.GCPtr** %128, align 8
  %130 = load i64, i64* %9, align 8
  %131 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %129, i64 %130
  %132 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %133 = load %struct.GCPtr*, %struct.GCPtr** %132, align 8
  %134 = load i64, i64* %6, align 8
  %135 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %133, i64 %134
  %136 = bitcast %struct.GCPtr* %135 to i8*
  %137 = bitcast %struct.GCPtr* %131 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %136, i8* align 8 %137, i64 28, i1 false)
  %138 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 0
  %139 = load %struct.GCPtr*, %struct.GCPtr** %138, align 8
  %140 = load i64, i64* %9, align 8
  %141 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %139, i64 %140
  call void @_ZN5GCPtr5clearEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %141)
  %142 = load i64, i64* %9, align 8
  store i64 %142, i64* %6, align 8
  %143 = load i64, i64* %9, align 8
  %144 = add i64 %143, 1
  %145 = getelementptr inbounds %struct.GC, %struct.GC* %10, i32 0, i32 2
  %146 = load i64, i64* %145, align 8
  %147 = urem i64 %144, %146
  store i64 %147, i64* %9, align 8
  br label %109, !llvm.loop !12

148:                                              ; preds = %14, %71, %95, %125
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZNK5GCPtr10isOccupiedEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %0) #0 comdat align 2 {
  %2 = alloca %struct.GCPtr*, align 8
  store %struct.GCPtr* %0, %struct.GCPtr** %2, align 8
  %3 = load %struct.GCPtr*, %struct.GCPtr** %2, align 8
  %4 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 2
  %5 = load i64, i64* %4, align 8
  %6 = icmp ne i64 %5, 0
  ret i1 %6
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN5GCPtr5clearEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %0) #0 comdat align 2 {
  %2 = alloca %struct.GCPtr*, align 8
  store %struct.GCPtr* %0, %struct.GCPtr** %2, align 8
  %3 = load %struct.GCPtr*, %struct.GCPtr** %2, align 8
  %4 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 0
  store i8* null, i8** %4, align 8
  %5 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 1
  store i64 0, i64* %5, align 8
  %6 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 2
  store i64 0, i64* %6, align 8
  %7 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %3, i32 0, i32 3
  store i32 0, i32* %7, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local noundef i32 @_ZN2GC6rehashEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i64 noundef %1) #2 align 2 {
  %3 = alloca i32, align 4
  %4 = alloca %struct.GC*, align 8
  %5 = alloca i64, align 8
  %6 = alloca %struct.GCPtr*, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %4, align 8
  store i64 %1, i64* %5, align 8
  %9 = load %struct.GC*, %struct.GC** %4, align 8
  %10 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 0
  %11 = load %struct.GCPtr*, %struct.GCPtr** %10, align 8
  store %struct.GCPtr* %11, %struct.GCPtr** %6, align 8
  %12 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 2
  %13 = load i64, i64* %12, align 8
  store i64 %13, i64* %7, align 8
  %14 = load i64, i64* %5, align 8
  %15 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 2
  store i64 %14, i64* %15, align 8
  %16 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 2
  %17 = load i64, i64* %16, align 8
  %18 = call noalias i8* @calloc(i64 noundef %17, i64 noundef 32) #9
  %19 = bitcast i8* %18 to %struct.GCPtr*
  %20 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 0
  store %struct.GCPtr* %19, %struct.GCPtr** %20, align 8
  %21 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 0
  %22 = load %struct.GCPtr*, %struct.GCPtr** %21, align 8
  %23 = icmp eq %struct.GCPtr* %22, null
  br i1 %23, label %24, label %29

24:                                               ; preds = %2
  %25 = load i64, i64* %7, align 8
  %26 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 2
  store i64 %25, i64* %26, align 8
  %27 = load %struct.GCPtr*, %struct.GCPtr** %6, align 8
  %28 = getelementptr inbounds %struct.GC, %struct.GC* %9, i32 0, i32 0
  store %struct.GCPtr* %27, %struct.GCPtr** %28, align 8
  store i32 0, i32* %3, align 4
  br label %59

29:                                               ; preds = %2
  store i64 0, i64* %8, align 8
  br label %30

30:                                               ; preds = %53, %29
  %31 = load i64, i64* %8, align 8
  %32 = load i64, i64* %7, align 8
  %33 = icmp ult i64 %31, %32
  br i1 %33, label %34, label %56

34:                                               ; preds = %30
  %35 = load %struct.GCPtr*, %struct.GCPtr** %6, align 8
  %36 = load i64, i64* %8, align 8
  %37 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %35, i64 %36
  %38 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %37, i32 0, i32 2
  %39 = load i64, i64* %38, align 8
  %40 = icmp ne i64 %39, 0
  br i1 %40, label %41, label %52

41:                                               ; preds = %34
  %42 = load %struct.GCPtr*, %struct.GCPtr** %6, align 8
  %43 = load i64, i64* %8, align 8
  %44 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %42, i64 %43
  %45 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %44, i32 0, i32 0
  %46 = load i8*, i8** %45, align 8
  %47 = load %struct.GCPtr*, %struct.GCPtr** %6, align 8
  %48 = load i64, i64* %8, align 8
  %49 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %47, i64 %48
  %50 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %49, i32 0, i32 1
  %51 = load i64, i64* %50, align 8
  call void @_ZN2GC6addPtrEPvm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %9, i8* noundef %46, i64 noundef %51)
  br label %52

52:                                               ; preds = %41, %34
  br label %53

53:                                               ; preds = %52
  %54 = load i64, i64* %8, align 8
  %55 = add i64 %54, 1
  store i64 %55, i64* %8, align 8
  br label %30, !llvm.loop !13

56:                                               ; preds = %30
  %57 = load %struct.GCPtr*, %struct.GCPtr** %6, align 8
  %58 = bitcast %struct.GCPtr* %57 to i8*
  call void @free(i8* noundef %58) #9
  store i32 1, i32* %3, align 4
  br label %59

59:                                               ; preds = %56, %24
  %60 = load i32, i32* %3, align 4
  ret i32 %60
}

; Function Attrs: nounwind
declare noalias i8* @calloc(i64 noundef, i64 noundef) #4

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC6addPtrEPvm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1, i64 noundef %2) #2 align 2 {
  %4 = alloca %struct.GC*, align 8
  %5 = alloca i8*, align 8
  %6 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %4, align 8
  store i8* %1, i8** %5, align 8
  store i64 %2, i64* %6, align 8
  %7 = load %struct.GC*, %struct.GC** %4, align 8
  %8 = load i8*, i8** %5, align 8
  %9 = ptrtoint i8* %8 to i64
  %10 = load i64, i64* %6, align 8
  %11 = add i64 %9, %10
  %12 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 7
  %13 = load i64, i64* %12, align 8
  %14 = icmp ugt i64 %11, %13
  br i1 %14, label %15, label %20

15:                                               ; preds = %3
  %16 = load i8*, i8** %5, align 8
  %17 = ptrtoint i8* %16 to i64
  %18 = load i64, i64* %6, align 8
  %19 = add i64 %17, %18
  br label %23

20:                                               ; preds = %3
  %21 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 7
  %22 = load i64, i64* %21, align 8
  br label %23

23:                                               ; preds = %20, %15
  %24 = phi i64 [ %19, %15 ], [ %22, %20 ]
  %25 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 7
  store i64 %24, i64* %25, align 8
  %26 = load i8*, i8** %5, align 8
  %27 = ptrtoint i8* %26 to i64
  %28 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 6
  %29 = load i64, i64* %28, align 8
  %30 = icmp ult i64 %27, %29
  br i1 %30, label %31, label %34

31:                                               ; preds = %23
  %32 = load i8*, i8** %5, align 8
  %33 = ptrtoint i8* %32 to i64
  br label %37

34:                                               ; preds = %23
  %35 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 6
  %36 = load i64, i64* %35, align 8
  br label %37

37:                                               ; preds = %34, %31
  %38 = phi i64 [ %33, %31 ], [ %36, %34 ]
  %39 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 6
  store i64 %38, i64* %39, align 8
  %40 = call noundef zeroext i1 @_ZN2GC10resizeMoreEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %7)
  br i1 %40, label %41, label %55

41:                                               ; preds = %37
  %42 = load i8*, i8** %5, align 8
  %43 = load i64, i64* %6, align 8
  call void @_ZN2GC10addPtrImplEPvm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %7, i8* noundef %42, i64 noundef %43)
  %44 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 8
  %45 = load i8, i8* %44, align 8
  %46 = trunc i8 %45 to i1
  br i1 %46, label %54, label %47

47:                                               ; preds = %41
  %48 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 3
  %49 = load i64, i64* %48, align 8
  %50 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 5
  %51 = load i64, i64* %50, align 8
  %52 = icmp ugt i64 %49, %51
  br i1 %52, label %53, label %54

53:                                               ; preds = %47
  call void @_ZN2GC3runEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %7)
  br label %54

54:                                               ; preds = %53, %47, %41
  br label %60

55:                                               ; preds = %37
  %56 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 3
  %57 = load i64, i64* %56, align 8
  %58 = add i64 %57, -1
  store i64 %58, i64* %56, align 8
  %59 = load i8*, i8** %5, align 8
  call void @free(i8* noundef %59) #9
  br label %60

60:                                               ; preds = %55, %54
  ret void
}

; Function Attrs: nounwind
declare void @free(i8* noundef) #4

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local noundef zeroext i1 @_ZN2GC10resizeMoreEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) #2 align 2 {
  %2 = alloca %struct.GC*, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %5 = load %struct.GC*, %struct.GC** %2, align 8
  %6 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 3
  %7 = load i64, i64* %6, align 8
  %8 = call noundef i64 @_ZN2GC12getPrimeSizeEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %5, i64 noundef %7)
  store i64 %8, i64* %3, align 8
  %9 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 2
  %10 = load i64, i64* %9, align 8
  store i64 %10, i64* %4, align 8
  %11 = load i64, i64* %3, align 8
  %12 = load i64, i64* %4, align 8
  %13 = icmp ugt i64 %11, %12
  br i1 %13, label %14, label %17

14:                                               ; preds = %1
  %15 = load i64, i64* %3, align 8
  %16 = call noundef i32 @_ZN2GC6rehashEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %5, i64 noundef %15)
  br label %18

17:                                               ; preds = %1
  br label %18

18:                                               ; preds = %17, %14
  %19 = phi i32 [ %16, %14 ], [ 1, %17 ]
  %20 = icmp ne i32 %19, 0
  ret i1 %20
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local noundef zeroext i1 @_ZN2GC10resizeLessEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) #2 align 2 {
  %2 = alloca %struct.GC*, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %5 = load %struct.GC*, %struct.GC** %2, align 8
  %6 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 3
  %7 = load i64, i64* %6, align 8
  %8 = call noundef i64 @_ZN2GC12getPrimeSizeEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %5, i64 noundef %7)
  store i64 %8, i64* %3, align 8
  %9 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 2
  %10 = load i64, i64* %9, align 8
  store i64 %10, i64* %4, align 8
  %11 = load i64, i64* %3, align 8
  %12 = load i64, i64* %4, align 8
  %13 = icmp ult i64 %11, %12
  br i1 %13, label %14, label %17

14:                                               ; preds = %1
  %15 = load i64, i64* %3, align 8
  %16 = call noundef i32 @_ZN2GC6rehashEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %5, i64 noundef %15)
  br label %18

17:                                               ; preds = %1
  br label %18

18:                                               ; preds = %17, %14
  %19 = phi i32 [ %16, %14 ], [ 1, %17 ]
  %20 = icmp ne i32 %19, 0
  ret i1 %20
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC7markPtrEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1) #2 align 2 {
  %3 = alloca %struct.GC*, align 8
  %4 = alloca i8*, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %3, align 8
  store i8* %1, i8** %4, align 8
  %8 = load %struct.GC*, %struct.GC** %3, align 8
  %9 = load i8*, i8** %4, align 8
  %10 = ptrtoint i8* %9 to i64
  %11 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 6
  %12 = load i64, i64* %11, align 8
  %13 = icmp ult i64 %10, %12
  br i1 %13, label %20, label %14

14:                                               ; preds = %2
  %15 = load i8*, i8** %4, align 8
  %16 = ptrtoint i8* %15 to i64
  %17 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 7
  %18 = load i64, i64* %17, align 8
  %19 = icmp ugt i64 %16, %18
  br i1 %19, label %20, label %21

20:                                               ; preds = %14, %2
  br label %115

21:                                               ; preds = %14
  %22 = load i8*, i8** %4, align 8
  %23 = call noundef i64 @_ZN2GC4hashEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %8, i8* noundef %22)
  %24 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 2
  %25 = load i64, i64* %24, align 8
  %26 = urem i64 %23, %25
  store i64 %26, i64* %5, align 8
  store i64 0, i64* %6, align 8
  br label %27

27:                                               ; preds = %21, %107
  %28 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %29 = load %struct.GCPtr*, %struct.GCPtr** %28, align 8
  %30 = load i64, i64* %5, align 8
  %31 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %29, i64 %30
  %32 = call noundef zeroext i1 @_ZNK5GCPtr7isEmptyEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %31)
  br i1 %32, label %44, label %33

33:                                               ; preds = %27
  %34 = load i64, i64* %6, align 8
  %35 = load i64, i64* %5, align 8
  %36 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %37 = load %struct.GCPtr*, %struct.GCPtr** %36, align 8
  %38 = load i64, i64* %5, align 8
  %39 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %37, i64 %38
  %40 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %39, i32 0, i32 2
  %41 = load i64, i64* %40, align 8
  %42 = call noundef i64 @_ZN2GC13probeDistanceEmm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %8, i64 noundef %35, i64 noundef %41)
  %43 = icmp ugt i64 %34, %42
  br i1 %43, label %44, label %45

44:                                               ; preds = %33, %27
  br label %115

45:                                               ; preds = %33
  %46 = load i8*, i8** %4, align 8
  %47 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %48 = load %struct.GCPtr*, %struct.GCPtr** %47, align 8
  %49 = load i64, i64* %5, align 8
  %50 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %48, i64 %49
  %51 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %50, i32 0, i32 0
  %52 = load i8*, i8** %51, align 8
  %53 = icmp eq i8* %46, %52
  br i1 %53, label %54, label %107

54:                                               ; preds = %45
  %55 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %56 = load %struct.GCPtr*, %struct.GCPtr** %55, align 8
  %57 = load i64, i64* %5, align 8
  %58 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %56, i64 %57
  %59 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %58, i32 0, i32 3
  %60 = load i32, i32* %59, align 8
  %61 = and i32 %60, 1
  %62 = icmp ne i32 %61, 0
  br i1 %62, label %63, label %64

63:                                               ; preds = %54
  br label %115

64:                                               ; preds = %54
  %65 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %66 = load %struct.GCPtr*, %struct.GCPtr** %65, align 8
  %67 = load i64, i64* %5, align 8
  %68 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %66, i64 %67
  %69 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %68, i32 0, i32 3
  %70 = load i32, i32* %69, align 8
  %71 = or i32 %70, 1
  store i32 %71, i32* %69, align 8
  %72 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %73 = load %struct.GCPtr*, %struct.GCPtr** %72, align 8
  %74 = load i64, i64* %5, align 8
  %75 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %73, i64 %74
  %76 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %75, i32 0, i32 3
  %77 = load i32, i32* %76, align 8
  %78 = and i32 %77, 4
  %79 = icmp ne i32 %78, 0
  br i1 %79, label %80, label %81

80:                                               ; preds = %64
  br label %115

81:                                               ; preds = %64
  store i64 0, i64* %7, align 8
  br label %82

82:                                               ; preds = %103, %81
  %83 = load i64, i64* %7, align 8
  %84 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %85 = load %struct.GCPtr*, %struct.GCPtr** %84, align 8
  %86 = load i64, i64* %5, align 8
  %87 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %85, i64 %86
  %88 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %87, i32 0, i32 1
  %89 = load i64, i64* %88, align 8
  %90 = udiv i64 %89, 8
  %91 = icmp ult i64 %83, %90
  br i1 %91, label %92, label %106

92:                                               ; preds = %82
  %93 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 0
  %94 = load %struct.GCPtr*, %struct.GCPtr** %93, align 8
  %95 = load i64, i64* %5, align 8
  %96 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %94, i64 %95
  %97 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %96, i32 0, i32 0
  %98 = load i8*, i8** %97, align 8
  %99 = bitcast i8* %98 to i8**
  %100 = load i64, i64* %7, align 8
  %101 = getelementptr inbounds i8*, i8** %99, i64 %100
  %102 = load i8*, i8** %101, align 8
  call void @_ZN2GC7markPtrEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %8, i8* noundef %102)
  br label %103

103:                                              ; preds = %92
  %104 = load i64, i64* %7, align 8
  %105 = add i64 %104, 1
  store i64 %105, i64* %7, align 8
  br label %82, !llvm.loop !14

106:                                              ; preds = %82
  br label %115

107:                                              ; preds = %45
  %108 = load i64, i64* %5, align 8
  %109 = add i64 %108, 1
  %110 = getelementptr inbounds %struct.GC, %struct.GC* %8, i32 0, i32 2
  %111 = load i64, i64* %110, align 8
  %112 = urem i64 %109, %111
  store i64 %112, i64* %5, align 8
  %113 = load i64, i64* %6, align 8
  %114 = add i64 %113, 1
  store i64 %114, i64* %6, align 8
  br label %27, !llvm.loop !15

115:                                              ; preds = %106, %80, %63, %44, %20
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC9markStackEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) #2 align 2 {
  %2 = alloca %struct.GC*, align 8
  %3 = alloca i8*, align 8
  %4 = alloca i8*, align 8
  %5 = alloca i8*, align 8
  %6 = alloca i8*, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %7 = load %struct.GC*, %struct.GC** %2, align 8
  %8 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 9
  %9 = load i8*, i8** %8, align 8
  store i8* %9, i8** %4, align 8
  %10 = bitcast i8** %3 to i8*
  store i8* %10, i8** %5, align 8
  %11 = load i8*, i8** %4, align 8
  %12 = load i8*, i8** %5, align 8
  %13 = icmp eq i8* %11, %12
  br i1 %13, label %14, label %15

14:                                               ; preds = %1
  br label %51

15:                                               ; preds = %1
  %16 = load i8*, i8** %4, align 8
  %17 = load i8*, i8** %5, align 8
  %18 = icmp ult i8* %16, %17
  br i1 %18, label %19, label %33

19:                                               ; preds = %15
  %20 = load i8*, i8** %5, align 8
  store i8* %20, i8** %6, align 8
  br label %21

21:                                               ; preds = %29, %19
  %22 = load i8*, i8** %6, align 8
  %23 = load i8*, i8** %4, align 8
  %24 = icmp uge i8* %22, %23
  br i1 %24, label %25, label %32

25:                                               ; preds = %21
  %26 = load i8*, i8** %6, align 8
  %27 = bitcast i8* %26 to i8**
  %28 = load i8*, i8** %27, align 8
  call void @_ZN2GC7markPtrEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %7, i8* noundef %28)
  br label %29

29:                                               ; preds = %25
  %30 = load i8*, i8** %6, align 8
  %31 = getelementptr inbounds i8, i8* %30, i64 -8
  store i8* %31, i8** %6, align 8
  br label %21, !llvm.loop !16

32:                                               ; preds = %21
  br label %33

33:                                               ; preds = %32, %15
  %34 = load i8*, i8** %4, align 8
  %35 = load i8*, i8** %5, align 8
  %36 = icmp ugt i8* %34, %35
  br i1 %36, label %37, label %51

37:                                               ; preds = %33
  %38 = load i8*, i8** %5, align 8
  store i8* %38, i8** %6, align 8
  br label %39

39:                                               ; preds = %47, %37
  %40 = load i8*, i8** %6, align 8
  %41 = load i8*, i8** %4, align 8
  %42 = icmp ule i8* %40, %41
  br i1 %42, label %43, label %50

43:                                               ; preds = %39
  %44 = load i8*, i8** %6, align 8
  %45 = bitcast i8* %44 to i8**
  %46 = load i8*, i8** %45, align 8
  call void @_ZN2GC7markPtrEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %7, i8* noundef %46)
  br label %47

47:                                               ; preds = %43
  %48 = load i8*, i8** %6, align 8
  %49 = getelementptr inbounds i8, i8* %48, i64 8
  store i8* %49, i8** %6, align 8
  br label %39, !llvm.loop !17

50:                                               ; preds = %39
  br label %51

51:                                               ; preds = %14, %50, %33
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC4markEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) #2 align 2 {
  %2 = alloca %struct.GC*, align 8
  %3 = alloca [1 x %struct.__jmp_buf_tag], align 16
  %4 = alloca { i64, i64 }, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %7 = load %struct.GC*, %struct.GC** %2, align 8
  store volatile { i64, i64 } { i64 ptrtoint (void (%struct.GC*)* @_ZN2GC9markStackEv to i64), i64 0 }, { i64, i64 }* %4, align 8
  %8 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 3
  %9 = load i64, i64* %8, align 8
  %10 = icmp eq i64 %9, 0
  br i1 %10, label %11, label %12

11:                                               ; preds = %1
  br label %116

12:                                               ; preds = %1
  store i64 0, i64* %5, align 8
  br label %13

13:                                               ; preds = %90, %12
  %14 = load i64, i64* %5, align 8
  %15 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 2
  %16 = load i64, i64* %15, align 8
  %17 = icmp ult i64 %14, %16
  br i1 %17, label %18, label %93

18:                                               ; preds = %13
  %19 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 0
  %20 = load %struct.GCPtr*, %struct.GCPtr** %19, align 8
  %21 = load i64, i64* %5, align 8
  %22 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %20, i64 %21
  %23 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %22, i32 0, i32 2
  %24 = load i64, i64* %23, align 8
  %25 = icmp eq i64 %24, 0
  br i1 %25, label %26, label %27

26:                                               ; preds = %18
  br label %90

27:                                               ; preds = %18
  %28 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 0
  %29 = load %struct.GCPtr*, %struct.GCPtr** %28, align 8
  %30 = load i64, i64* %5, align 8
  %31 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %29, i64 %30
  %32 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %31, i32 0, i32 3
  %33 = load i32, i32* %32, align 8
  %34 = and i32 %33, 1
  %35 = icmp ne i32 %34, 0
  br i1 %35, label %36, label %37

36:                                               ; preds = %27
  br label %90

37:                                               ; preds = %27
  %38 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 0
  %39 = load %struct.GCPtr*, %struct.GCPtr** %38, align 8
  %40 = load i64, i64* %5, align 8
  %41 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %39, i64 %40
  %42 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %41, i32 0, i32 3
  %43 = load i32, i32* %42, align 8
  %44 = and i32 %43, 2
  %45 = icmp ne i32 %44, 0
  br i1 %45, label %46, label %89

46:                                               ; preds = %37
  %47 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 0
  %48 = load %struct.GCPtr*, %struct.GCPtr** %47, align 8
  %49 = load i64, i64* %5, align 8
  %50 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %48, i64 %49
  %51 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %50, i32 0, i32 3
  %52 = load i32, i32* %51, align 8
  %53 = or i32 %52, 1
  store i32 %53, i32* %51, align 8
  %54 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 0
  %55 = load %struct.GCPtr*, %struct.GCPtr** %54, align 8
  %56 = load i64, i64* %5, align 8
  %57 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %55, i64 %56
  %58 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %57, i32 0, i32 3
  %59 = load i32, i32* %58, align 8
  %60 = and i32 %59, 4
  %61 = icmp ne i32 %60, 0
  br i1 %61, label %62, label %63

62:                                               ; preds = %46
  br label %90

63:                                               ; preds = %46
  store i64 0, i64* %6, align 8
  br label %64

64:                                               ; preds = %85, %63
  %65 = load i64, i64* %6, align 8
  %66 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 0
  %67 = load %struct.GCPtr*, %struct.GCPtr** %66, align 8
  %68 = load i64, i64* %5, align 8
  %69 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %67, i64 %68
  %70 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %69, i32 0, i32 1
  %71 = load i64, i64* %70, align 8
  %72 = udiv i64 %71, 8
  %73 = icmp ult i64 %65, %72
  br i1 %73, label %74, label %88

74:                                               ; preds = %64
  %75 = getelementptr inbounds %struct.GC, %struct.GC* %7, i32 0, i32 0
  %76 = load %struct.GCPtr*, %struct.GCPtr** %75, align 8
  %77 = load i64, i64* %5, align 8
  %78 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %76, i64 %77
  %79 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %78, i32 0, i32 0
  %80 = load i8*, i8** %79, align 8
  %81 = bitcast i8* %80 to i8**
  %82 = load i64, i64* %6, align 8
  %83 = getelementptr inbounds i8*, i8** %81, i64 %82
  %84 = load i8*, i8** %83, align 8
  call void @_ZN2GC7markPtrEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %7, i8* noundef %84)
  br label %85

85:                                               ; preds = %74
  %86 = load i64, i64* %6, align 8
  %87 = add i64 %86, 1
  store i64 %87, i64* %6, align 8
  br label %64, !llvm.loop !18

88:                                               ; preds = %64
  br label %90

89:                                               ; preds = %37
  br label %90

90:                                               ; preds = %89, %88, %62, %36, %26
  %91 = load i64, i64* %5, align 8
  %92 = add i64 %91, 1
  store i64 %92, i64* %5, align 8
  br label %13, !llvm.loop !19

93:                                               ; preds = %13
  %94 = bitcast [1 x %struct.__jmp_buf_tag]* %3 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %94, i8 0, i64 200, i1 false)
  %95 = getelementptr inbounds [1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag]* %3, i64 0, i64 0
  %96 = call i32 @_setjmp(%struct.__jmp_buf_tag* noundef %95) #10
  %97 = load volatile { i64, i64 }, { i64, i64 }* %4, align 8
  %98 = extractvalue { i64, i64 } %97, 1
  %99 = bitcast %struct.GC* %7 to i8*
  %100 = getelementptr inbounds i8, i8* %99, i64 %98
  %101 = bitcast i8* %100 to %struct.GC*
  %102 = extractvalue { i64, i64 } %97, 0
  %103 = and i64 %102, 1
  %104 = icmp ne i64 %103, 0
  br i1 %104, label %105, label %112

105:                                              ; preds = %93
  %106 = bitcast %struct.GC* %101 to i8**
  %107 = load i8*, i8** %106, align 8
  %108 = sub i64 %102, 1
  %109 = getelementptr i8, i8* %107, i64 %108, !nosanitize !20
  %110 = bitcast i8* %109 to void (%struct.GC*)**, !nosanitize !20
  %111 = load void (%struct.GC*)*, void (%struct.GC*)** %110, align 8, !nosanitize !20
  br label %114

112:                                              ; preds = %93
  %113 = inttoptr i64 %102 to void (%struct.GC*)*
  br label %114

114:                                              ; preds = %112, %105
  %115 = phi void (%struct.GC*)* [ %111, %105 ], [ %113, %112 ]
  call void %115(%struct.GC* noundef nonnull align 8 dereferenceable(88) %101)
  br label %116

116:                                              ; preds = %114, %11
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #5

; Function Attrs: nounwind returns_twice
declare i32 @_setjmp(%struct.__jmp_buf_tag* noundef) #6

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC5sweepEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) #2 align 2 {
  %2 = alloca %struct.GC*, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca %struct.GCPtr, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %13 = load %struct.GC*, %struct.GC** %2, align 8
  %14 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 3
  %15 = load i64, i64* %14, align 8
  %16 = icmp eq i64 %15, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %1
  br label %239

18:                                               ; preds = %1
  %19 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 4
  store i64 0, i64* %19, align 8
  store i64 0, i64* %5, align 8
  br label %20

20:                                               ; preds = %58, %18
  %21 = load i64, i64* %5, align 8
  %22 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 2
  %23 = load i64, i64* %22, align 8
  %24 = icmp ult i64 %21, %23
  br i1 %24, label %25, label %61

25:                                               ; preds = %20
  %26 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %27 = load %struct.GCPtr*, %struct.GCPtr** %26, align 8
  %28 = load i64, i64* %5, align 8
  %29 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %27, i64 %28
  %30 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %29, i32 0, i32 2
  %31 = load i64, i64* %30, align 8
  %32 = icmp eq i64 %31, 0
  br i1 %32, label %33, label %34

33:                                               ; preds = %25
  br label %58

34:                                               ; preds = %25
  %35 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %36 = load %struct.GCPtr*, %struct.GCPtr** %35, align 8
  %37 = load i64, i64* %5, align 8
  %38 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %36, i64 %37
  %39 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %38, i32 0, i32 3
  %40 = load i32, i32* %39, align 8
  %41 = and i32 %40, 1
  %42 = icmp ne i32 %41, 0
  br i1 %42, label %43, label %44

43:                                               ; preds = %34
  br label %58

44:                                               ; preds = %34
  %45 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %46 = load %struct.GCPtr*, %struct.GCPtr** %45, align 8
  %47 = load i64, i64* %5, align 8
  %48 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %46, i64 %47
  %49 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %48, i32 0, i32 3
  %50 = load i32, i32* %49, align 8
  %51 = and i32 %50, 2
  %52 = icmp ne i32 %51, 0
  br i1 %52, label %53, label %54

53:                                               ; preds = %44
  br label %58

54:                                               ; preds = %44
  %55 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 4
  %56 = load i64, i64* %55, align 8
  %57 = add i64 %56, 1
  store i64 %57, i64* %55, align 8
  br label %58

58:                                               ; preds = %54, %53, %43, %33
  %59 = load i64, i64* %5, align 8
  %60 = add i64 %59, 1
  store i64 %60, i64* %5, align 8
  br label %20, !llvm.loop !21

61:                                               ; preds = %20
  %62 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 1
  %63 = load %struct.GCPtr*, %struct.GCPtr** %62, align 8
  %64 = bitcast %struct.GCPtr* %63 to i8*
  %65 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 4
  %66 = load i64, i64* %65, align 8
  %67 = mul i64 32, %66
  %68 = call i8* @realloc(i8* noundef %64, i64 noundef %67) #9
  %69 = bitcast i8* %68 to %struct.GCPtr*
  %70 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 1
  store %struct.GCPtr* %69, %struct.GCPtr** %70, align 8
  store i64 0, i64* %6, align 8
  store i64 0, i64* %7, align 8
  br label %71

71:                                               ; preds = %156, %61
  %72 = load i64, i64* %7, align 8
  %73 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 2
  %74 = load i64, i64* %73, align 8
  %75 = icmp ult i64 %72, %74
  br i1 %75, label %76, label %159

76:                                               ; preds = %71
  %77 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %78 = load %struct.GCPtr*, %struct.GCPtr** %77, align 8
  %79 = load i64, i64* %7, align 8
  %80 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %78, i64 %79
  %81 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %80, i32 0, i32 2
  %82 = load i64, i64* %81, align 8
  %83 = icmp eq i64 %82, 0
  br i1 %83, label %84, label %85

84:                                               ; preds = %76
  br label %156

85:                                               ; preds = %76
  %86 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %87 = load %struct.GCPtr*, %struct.GCPtr** %86, align 8
  %88 = load i64, i64* %7, align 8
  %89 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %87, i64 %88
  %90 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %89, i32 0, i32 3
  %91 = load i32, i32* %90, align 8
  %92 = and i32 %91, 1
  %93 = icmp ne i32 %92, 0
  br i1 %93, label %94, label %95

94:                                               ; preds = %85
  br label %156

95:                                               ; preds = %85
  %96 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %97 = load %struct.GCPtr*, %struct.GCPtr** %96, align 8
  %98 = load i64, i64* %7, align 8
  %99 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %97, i64 %98
  %100 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %99, i32 0, i32 3
  %101 = load i32, i32* %100, align 8
  %102 = and i32 %101, 2
  %103 = icmp ne i32 %102, 0
  br i1 %103, label %104, label %105

104:                                              ; preds = %95
  br label %156

105:                                              ; preds = %95
  %106 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %107 = load %struct.GCPtr*, %struct.GCPtr** %106, align 8
  %108 = load i64, i64* %7, align 8
  %109 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %107, i64 %108
  %110 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 1
  %111 = load %struct.GCPtr*, %struct.GCPtr** %110, align 8
  %112 = load i64, i64* %6, align 8
  %113 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %111, i64 %112
  %114 = bitcast %struct.GCPtr* %113 to i8*
  %115 = bitcast %struct.GCPtr* %109 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %114, i8* align 8 %115, i64 28, i1 false)
  %116 = load i64, i64* %6, align 8
  %117 = add i64 %116, 1
  store i64 %117, i64* %6, align 8
  %118 = load i64, i64* %7, align 8
  store i64 %118, i64* %8, align 8
  br label %119

119:                                              ; preds = %105, %151
  %120 = load i64, i64* %8, align 8
  %121 = add i64 %120, 1
  %122 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 2
  %123 = load i64, i64* %122, align 8
  %124 = urem i64 %121, %123
  store i64 %124, i64* %9, align 8
  %125 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %126 = load %struct.GCPtr*, %struct.GCPtr** %125, align 8
  %127 = load i64, i64* %9, align 8
  %128 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %126, i64 %127
  %129 = bitcast %struct.GCPtr* %10 to i8*
  %130 = bitcast %struct.GCPtr* %128 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %129, i8* align 8 %130, i64 32, i1 false)
  %131 = call noundef zeroext i1 @_ZNK5GCPtr10isOccupiedEv(%struct.GCPtr* noundef nonnull align 8 dereferenceable(28) %10)
  br i1 %131, label %132, label %150

132:                                              ; preds = %119
  %133 = load i64, i64* %9, align 8
  %134 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %10, i32 0, i32 2
  %135 = load i64, i64* %134, align 8
  %136 = call noundef i64 @_ZN2GC13probeDistanceEmm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %13, i64 noundef %133, i64 noundef %135)
  %137 = icmp ugt i64 %136, 0
  br i1 %137, label %138, label %150

138:                                              ; preds = %132
  %139 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %140 = load %struct.GCPtr*, %struct.GCPtr** %139, align 8
  %141 = load i64, i64* %9, align 8
  %142 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %140, i64 %141
  %143 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %144 = load %struct.GCPtr*, %struct.GCPtr** %143, align 8
  %145 = load i64, i64* %8, align 8
  %146 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %144, i64 %145
  %147 = bitcast %struct.GCPtr* %146 to i8*
  %148 = bitcast %struct.GCPtr* %142 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %147, i8* align 8 %148, i64 28, i1 false)
  %149 = load i64, i64* %9, align 8
  store i64 %149, i64* %8, align 8
  br label %151

150:                                              ; preds = %132, %119
  br label %152

151:                                              ; preds = %138
  br label %119, !llvm.loop !22

152:                                              ; preds = %150
  %153 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 3
  %154 = load i64, i64* %153, align 8
  %155 = add i64 %154, -1
  store i64 %155, i64* %153, align 8
  br label %156

156:                                              ; preds = %152, %104, %94, %84
  %157 = load i64, i64* %7, align 8
  %158 = add i64 %157, 1
  store i64 %158, i64* %7, align 8
  br label %71, !llvm.loop !23

159:                                              ; preds = %71
  store i64 0, i64* %11, align 8
  br label %160

160:                                              ; preds = %192, %159
  %161 = load i64, i64* %11, align 8
  %162 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 2
  %163 = load i64, i64* %162, align 8
  %164 = icmp ult i64 %161, %163
  br i1 %164, label %165, label %195

165:                                              ; preds = %160
  %166 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %167 = load %struct.GCPtr*, %struct.GCPtr** %166, align 8
  %168 = load i64, i64* %11, align 8
  %169 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %167, i64 %168
  %170 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %169, i32 0, i32 2
  %171 = load i64, i64* %170, align 8
  %172 = icmp eq i64 %171, 0
  br i1 %172, label %173, label %174

173:                                              ; preds = %165
  br label %192

174:                                              ; preds = %165
  %175 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %176 = load %struct.GCPtr*, %struct.GCPtr** %175, align 8
  %177 = load i64, i64* %11, align 8
  %178 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %176, i64 %177
  %179 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %178, i32 0, i32 3
  %180 = load i32, i32* %179, align 8
  %181 = and i32 %180, 1
  %182 = icmp ne i32 %181, 0
  br i1 %182, label %183, label %191

183:                                              ; preds = %174
  %184 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 0
  %185 = load %struct.GCPtr*, %struct.GCPtr** %184, align 8
  %186 = load i64, i64* %11, align 8
  %187 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %185, i64 %186
  %188 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %187, i32 0, i32 3
  %189 = load i32, i32* %188, align 8
  %190 = and i32 %189, -2
  store i32 %190, i32* %188, align 8
  br label %191

191:                                              ; preds = %183, %174
  br label %192

192:                                              ; preds = %191, %173
  %193 = load i64, i64* %11, align 8
  %194 = add i64 %193, 1
  store i64 %194, i64* %11, align 8
  br label %160, !llvm.loop !24

195:                                              ; preds = %160
  %196 = call noundef zeroext i1 @_ZN2GC10resizeLessEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %13)
  %197 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 3
  %198 = load i64, i64* %197, align 8
  %199 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 3
  %200 = load i64, i64* %199, align 8
  %201 = uitofp i64 %200 to double
  %202 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 10
  %203 = load double, double* %202, align 8
  %204 = fmul double %201, %203
  %205 = fptoui double %204 to i64
  %206 = add i64 %198, %205
  %207 = add i64 %206, 1
  %208 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 5
  store i64 %207, i64* %208, align 8
  store i64 0, i64* %12, align 8
  br label %209

209:                                              ; preds = %230, %195
  %210 = load i64, i64* %12, align 8
  %211 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 4
  %212 = load i64, i64* %211, align 8
  %213 = icmp ult i64 %210, %212
  br i1 %213, label %214, label %233

214:                                              ; preds = %209
  %215 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 1
  %216 = load %struct.GCPtr*, %struct.GCPtr** %215, align 8
  %217 = load i64, i64* %12, align 8
  %218 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %216, i64 %217
  %219 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %218, i32 0, i32 0
  %220 = load i8*, i8** %219, align 8
  %221 = icmp ne i8* %220, null
  br i1 %221, label %222, label %229

222:                                              ; preds = %214
  %223 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 1
  %224 = load %struct.GCPtr*, %struct.GCPtr** %223, align 8
  %225 = load i64, i64* %12, align 8
  %226 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %224, i64 %225
  %227 = getelementptr inbounds %struct.GCPtr, %struct.GCPtr* %226, i32 0, i32 0
  %228 = load i8*, i8** %227, align 8
  call void @free(i8* noundef %228) #9
  br label %229

229:                                              ; preds = %222, %214
  br label %230

230:                                              ; preds = %229
  %231 = load i64, i64* %12, align 8
  %232 = add i64 %231, 1
  store i64 %232, i64* %12, align 8
  br label %209, !llvm.loop !25

233:                                              ; preds = %209
  %234 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 1
  %235 = load %struct.GCPtr*, %struct.GCPtr** %234, align 8
  %236 = bitcast %struct.GCPtr* %235 to i8*
  call void @free(i8* noundef %236) #9
  %237 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 1
  store %struct.GCPtr* null, %struct.GCPtr** %237, align 8
  %238 = getelementptr inbounds %struct.GC, %struct.GC* %13, i32 0, i32 4
  store i64 0, i64* %238, align 8
  br label %239

239:                                              ; preds = %233, %17
  ret void
}

; Function Attrs: nounwind
declare i8* @realloc(i8* noundef, i64 noundef) #4

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC3runEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) #2 align 2 {
  %2 = alloca %struct.GC*, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %3 = load %struct.GC*, %struct.GC** %2, align 8
  call void @_ZN2GC4markEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %3)
  call void @_ZN2GC5sweepEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %3)
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC9removePtrEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1) #2 align 2 {
  %3 = alloca %struct.GC*, align 8
  %4 = alloca i8*, align 8
  store %struct.GC* %0, %struct.GC** %3, align 8
  store i8* %1, i8** %4, align 8
  %5 = load %struct.GC*, %struct.GC** %3, align 8
  %6 = load i8*, i8** %4, align 8
  call void @_ZN2GC13removePtrImplEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %5, i8* noundef %6)
  %7 = call noundef zeroext i1 @_ZN2GC10resizeLessEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %5)
  %8 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 3
  %9 = load i64, i64* %8, align 8
  %10 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 3
  %11 = load i64, i64* %10, align 8
  %12 = udiv i64 %11, 2
  %13 = add i64 %9, %12
  %14 = add i64 %13, 1
  %15 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 5
  store i64 %14, i64* %15, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_ZN2GC5startEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i8* noundef %1) #0 align 2 {
  %3 = alloca %struct.GC*, align 8
  %4 = alloca i8*, align 8
  store %struct.GC* %0, %struct.GC** %3, align 8
  store i8* %1, i8** %4, align 8
  %5 = load %struct.GC*, %struct.GC** %3, align 8
  %6 = load i8*, i8** %4, align 8
  %7 = getelementptr inbounds %struct.GC, %struct.GC* %5, i32 0, i32 9
  store i8* %6, i8** %7, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_ZN2GC4stopEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) #2 align 2 {
  %2 = alloca %struct.GC*, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %3 = load %struct.GC*, %struct.GC** %2, align 8
  call void @_ZN2GC5sweepEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) %3)
  %4 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 0
  %5 = load %struct.GCPtr*, %struct.GCPtr** %4, align 8
  %6 = bitcast %struct.GCPtr* %5 to i8*
  call void @free(i8* noundef %6) #9
  %7 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 1
  %8 = load %struct.GCPtr*, %struct.GCPtr** %7, align 8
  %9 = bitcast %struct.GCPtr* %8 to i8*
  call void @free(i8* noundef %9) #9
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local noundef i8* @_ZN2GC5allocEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0, i64 noundef %1) #2 align 2 {
  %3 = alloca %struct.GC*, align 8
  %4 = alloca i64, align 8
  %5 = alloca i8*, align 8
  store %struct.GC* %0, %struct.GC** %3, align 8
  store i64 %1, i64* %4, align 8
  %6 = load %struct.GC*, %struct.GC** %3, align 8
  %7 = load i64, i64* %4, align 8
  %8 = call noalias i8* @malloc(i64 noundef %7) #9
  store i8* %8, i8** %5, align 8
  %9 = load i8*, i8** %5, align 8
  %10 = icmp ne i8* %9, null
  br i1 %10, label %11, label %14

11:                                               ; preds = %2
  %12 = load i8*, i8** %5, align 8
  %13 = load i64, i64* %4, align 8
  call void @_ZN2GC6addPtrEPvm(%struct.GC* noundef nonnull align 8 dereferenceable(88) %6, i8* noundef %12, i64 noundef %13)
  br label %14

14:                                               ; preds = %11, %2
  %15 = load i8*, i8** %5, align 8
  ret i8* %15
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64 noundef) #4

; Function Attrs: noinline uwtable
define internal void @__cxx_global_var_init() #7 section ".text.startup" {
  call void @_ZN2GCC2Ev(%struct.GC* noundef nonnull align 8 dereferenceable(88) @_ZL2gc) #9
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN2GCC2Ev(%struct.GC* noundef nonnull align 8 dereferenceable(88) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca %struct.GC*, align 8
  store %struct.GC* %0, %struct.GC** %2, align 8
  %3 = load %struct.GC*, %struct.GC** %2, align 8
  %4 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 2
  store i64 0, i64* %4, align 8
  %5 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 3
  store i64 0, i64* %5, align 8
  %6 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 4
  store i64 0, i64* %6, align 8
  %7 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 5
  store i64 0, i64* %7, align 8
  %8 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 6
  store i64 -1, i64* %8, align 8
  %9 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 7
  store i64 0, i64* %9, align 8
  %10 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 8
  store i8 0, i8* %10, align 8
  %11 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 9
  store i8* null, i8** %11, align 8
  %12 = getelementptr inbounds %struct.GC, %struct.GC* %3, i32 0, i32 10
  store double 5.000000e-01, double* %12, align 8
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z16example_functionv() #2 {
  %1 = alloca %struct.A*, align 8
  %2 = alloca %struct.B*, align 8
  %3 = call noundef i8* @_ZN2GC5allocEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) @_ZL2gc, i64 noundef 8)
  %4 = bitcast i8* %3 to %struct.A*
  store %struct.A* %4, %struct.A** %1, align 8
  %5 = call noundef i8* @_ZN2GC5allocEm(%struct.GC* noundef nonnull align 8 dereferenceable(88) @_ZL2gc, i64 noundef 8)
  %6 = bitcast i8* %5 to %struct.B*
  store %struct.B* %6, %struct.B** %2, align 8
  %7 = load %struct.B*, %struct.B** %2, align 8
  %8 = load %struct.A*, %struct.A** %1, align 8
  %9 = getelementptr inbounds %struct.A, %struct.A* %8, i32 0, i32 0
  store %struct.B* %7, %struct.B** %9, align 8
  %10 = load %struct.A*, %struct.A** %1, align 8
  %11 = load %struct.B*, %struct.B** %2, align 8
  %12 = getelementptr inbounds %struct.B, %struct.B* %11, i32 0, i32 0
  store %struct.A* %10, %struct.A** %12, align 8
  ret void
}

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main(i32 noundef %0, i8** noundef %1) #8 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  %6 = bitcast i32* %4 to i8*
  call void @_ZN2GC5startEPv(%struct.GC* noundef nonnull align 8 dereferenceable(88) @_ZL2gc, i8* noundef %6)
  call void @_Z16example_functionv()
  call void @_ZN2GC4stopEv(%struct.GC* noundef nonnull align 8 dereferenceable(88) @_ZL2gc)
  ret i32 0
}

; Function Attrs: noinline uwtable
define internal void @_GLOBAL__sub_I_gc.cpp() #7 section ".text.startup" {
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nofree nounwind willreturn }
attributes #2 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { argmemonly nofree nounwind willreturn writeonly }
attributes #6 = { nounwind returns_twice "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #7 = { noinline uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nounwind }
attributes #10 = { nounwind returns_twice }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 14.0.0-1ubuntu1.1"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}
!12 = distinct !{!12, !7}
!13 = distinct !{!13, !7}
!14 = distinct !{!14, !7}
!15 = distinct !{!15, !7}
!16 = distinct !{!16, !7}
!17 = distinct !{!17, !7}
!18 = distinct !{!18, !7}
!19 = distinct !{!19, !7}
!20 = !{}
!21 = distinct !{!21, !7}
!22 = distinct !{!22, !7}
!23 = distinct !{!23, !7}
!24 = distinct !{!24, !7}
!25 = distinct !{!25, !7}
