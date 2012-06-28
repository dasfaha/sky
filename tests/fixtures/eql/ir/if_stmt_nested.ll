; ModuleID = 'foo'

define i64 @main() {
  %bar = alloca i64
  %foo = alloca i64
  store i64 0, i64* %bar
  %1 = load i64* %bar
  %2 = icmp eq i64 %1, 1
  br i1 %2, label %3, label %14

; <label>:3                                       ; preds = %0
  %4 = load i64* %bar
  %5 = icmp eq i64 %4, 10
  br i1 %5, label %6, label %7

; <label>:6                                       ; preds = %3
  store i64 1, i64* %foo
  br label %13

; <label>:7                                       ; preds = %3
  %8 = load i64* %bar
  %9 = icmp eq i64 %8, 11
  br i1 %9, label %10, label %11

; <label>:10                                      ; preds = %7
  store i64 2, i64* %foo
  br label %12

; <label>:11                                      ; preds = %7
  store i64 3, i64* %foo
  br label %12

; <label>:12                                      ; preds = %11, %10
  br label %13

; <label>:13                                      ; preds = %12, %6
  br label %20

; <label>:14                                      ; preds = %0
  %15 = load i64* %bar
  %16 = icmp eq i64 %15, 2
  br i1 %16, label %17, label %18

; <label>:17                                      ; preds = %14
  store i64 4, i64* %foo
  br label %19

; <label>:18                                      ; preds = %14
  store i64 5, i64* %foo
  br label %19

; <label>:19                                      ; preds = %18, %17
  br label %20

; <label>:20                                      ; preds = %19, %13
  %21 = load i64* %foo
  ret i64 %21
}
