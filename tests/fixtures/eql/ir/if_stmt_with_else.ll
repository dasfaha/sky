; ModuleID = 'foo'

define i64 @main() {
entry:
  %bar = alloca i64
  %foo = alloca i64
  store i64 20, i64* %bar
  %0 = load i64* %bar
  %1 = icmp eq i64 %0, 20
  br i1 %1, label %2, label %3

; <label>:2                                       ; preds = %entry
  store i64 1, i64* %foo
  br label %4

; <label>:3                                       ; preds = %entry
  store i64 0, i64* %foo
  br label %4

; <label>:4                                       ; preds = %3, %2
  %5 = load i64* %foo
  ret i64 %5
}
