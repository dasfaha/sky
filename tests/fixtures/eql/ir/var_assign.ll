; ModuleID = 'foo'

define i64 @main() {
entry:
  %foo = alloca i64
  store i64 20, i64* %foo
  %0 = load i64* %foo
  ret i64 %0
}
