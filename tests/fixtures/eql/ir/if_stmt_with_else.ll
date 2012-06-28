; ModuleID = 'foo'

define i64 @main() {
  %bar = alloca i64
  %foo = alloca i64
  store i64 20, i64* %bar
  %1 = load i64* %bar
  %2 = icmp eq i64 %1, 20
  br i1 %2, label %3, label %4

; <label>:3                                       ; preds = %0
  store i64 1, i64* %foo
  br label %5

; <label>:4                                       ; preds = %0
  store i64 0, i64* %foo
  br label %5

; <label>:5                                       ; preds = %4, %3
  %6 = load i64* %foo
  ret i64 %6
}
