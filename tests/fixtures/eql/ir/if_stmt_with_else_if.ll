; ModuleID = 'foo'

define i64 @main() {
  %bar = alloca i64
  %foo = alloca i64
  store i64 0, i64* %bar
  %1 = load i64* %bar
  %2 = icmp eq i64 %1, 1
  br i1 %2, label %3, label %4

; <label>:3                                       ; preds = %0
  store i64 1, i64* %foo
  br label %9

; <label>:4                                       ; preds = %0
  %5 = load i64* %bar
  %6 = icmp eq i64 %5, 2
  br i1 %6, label %7, label %8

; <label>:7                                       ; preds = %4
  store i64 2, i64* %foo
  br label %8

; <label>:8                                       ; preds = %7, %4
  br label %9

; <label>:9                                       ; preds = %8, %3
  %10 = load i64* %foo
  ret i64 %10
}
