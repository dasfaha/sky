; ModuleID = 'foo'

define i64 @main() {
entry:
  %bar = alloca i64
  %foo = alloca i64
  store i64 0, i64* %bar
  %0 = load i64* %bar
  %1 = icmp eq i64 %0, 1
  br i1 %1, label %2, label %3

; <label>:2                                       ; preds = %entry
  store i64 1, i64* %foo
  br label %8

; <label>:3                                       ; preds = %entry
  %4 = load i64* %bar
  %5 = icmp eq i64 %4, 2
  br i1 %5, label %6, label %7

; <label>:6                                       ; preds = %3
  store i64 2, i64* %foo
  br label %7

; <label>:7                                       ; preds = %6, %3
  br label %8

; <label>:8                                       ; preds = %7, %2
  %9 = load i64* %foo
  ret i64 %9
}
