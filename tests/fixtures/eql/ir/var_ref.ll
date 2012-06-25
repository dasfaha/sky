; ModuleID = 'foo'

define i64 @main() {
entry:
  %bar = alloca double
  %foo = alloca i64
  store i64 200, i64* %foo
  %0 = load i64* %foo
  ret i64 %0
}
